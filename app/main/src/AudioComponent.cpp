
#include "AudioComponent.h"
#include <queue>          // std::queue

extern "C" {


}

/*---------------------------------------------------------------------------------------------------------*/
/* Function:        app_dev_create_signal_flow                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t AudioComponent::app_dev_create_signal_flow()
{

	dsp_buffers_pool = memory_pool_init(20, I2S_BUFF_LEN * sizeof(float));
	pMain_dsp_chain = DSP_CREATE_CHAIN(18 , dsp_buffers_pool);

	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, MIXER2X1_API_MODULE_NAME, &stereo_to_mono);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, WEBRTC_VOICE_ACTIVITY_DETECTION_API_MODULE_NAME, &vad);



	/**************** module links  **********************/
	/******** LF path **********/
	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain, DSP_INPUT_PAD_0, &stereo_to_mono, DSP_INPUT_PAD_0);
	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain, DSP_INPUT_PAD_0, &stereo_to_mono, DSP_INPUT_PAD_1);

	DSP_CREATE_INTER_MODULES_LINK(&stereo_to_mono, DSP_OUTPUT_PAD_0, &vad, DSP_INPUT_PAD_0);


	DSP_CREATE_MODULE_TO_CHAIN_OUTPUT_LINK(pMain_dsp_chain, DSP_OUTPUT_PAD_0, &stereo_to_mono, DSP_OUTPUT_PAD_0);
	DSP_CREATE_MODULE_TO_CHAIN_OUTPUT_LINK(pMain_dsp_chain, DSP_OUTPUT_PAD_1, &stereo_to_mono, DSP_OUTPUT_PAD_0);

	return 0;
}



AudioComponent::AudioComponent() 
 : Thread("Background Thread")
{

	set_channel_weight_t ch_weight;

	addControlCommands();

	mixer2x1_init();
	webrtc_voice_activity_detection_init();




	app_dev_create_signal_flow();

#if 1
	/**************   LPF path  *************/
	ch_weight.weight = 0.5;
	ch_weight.channel_num = 0;
	DSP_IOCTL_1_PARAMS(&stereo_to_mono, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	ch_weight.channel_num = 1;
	DSP_IOCTL_1_PARAMS(&stereo_to_mono, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	//			dsp_management_api_set_module_control(&stereo_to_mono , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

#endif



	formatManager.registerBasicFormats();       // [1]
	transportSource.addChangeListener(this);   // [2]

	startThread();

}

AudioComponent::~AudioComponent()
{
	stopThread(1000);
}

void AudioComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) 
{
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}


void AudioComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) 
{
	if (readerSource == nullptr)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}

	transportSource.getNextAudioBlock(bufferToFill);
#if 0
	const float level = (float)levelSlider.getValue();

	for (int channel = 0; channel < bufferToFill.buffer->getNumChannels(); ++channel)
	{
		// Get a pointer to the start sample in the buffer for this audio output channel
		float* const buffer = bufferToFill.buffer->getWritePointer(channel, bufferToFill.startSample);

		// Fill the required number of samples with noise betweem -0.125 and +0.125
		for (int sample = 0; sample < bufferToFill.numSamples; ++sample)
		{
			const float noise = random.nextFloat() * 2.0f - 1.0f;
			buffer[sample] += noise * level;
		}
	}
#else
	uint32_t buff_len;
	float* buffer1 = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
	float* buffer2 = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);	
	float *buffer_out1;
	float *buffer_out2;
	buff_len = bufferToFill.numSamples;


	buffer_out1 = (float *)malloc(buff_len*sizeof(float));
	buffer_out2 = (float *)malloc(buff_len*sizeof(float));
	DSP_SET_CHAIN_INPUT_BUFFER(pMain_dsp_chain, DSP_INPUT_PAD_0, (float*)buffer1);
	DSP_SET_CHAIN_INPUT_BUFFER(pMain_dsp_chain, DSP_INPUT_PAD_1, (float*)buffer2);
	DSP_SET_CHAIN_OUTPUT_BUFFER(pMain_dsp_chain, DSP_OUTPUT_PAD_0, (float*)buffer_out1);
	DSP_SET_CHAIN_OUTPUT_BUFFER(pMain_dsp_chain, DSP_OUTPUT_PAD_1, (float*)buffer_out2);
	DSP_PROCESS_CHAIN(pMain_dsp_chain, buff_len);
	//memcpy(buffer1, buffer_out1, buff_len*sizeof(float));  // keep the same buffer .....
	//memcpy(buffer2, buffer_out2, buff_len*sizeof(float));
	free(buffer_out1);
	free(buffer_out2);

#endif

}

void AudioComponent::releaseResources() 
{
	transportSource.releaseResources();
}



void AudioComponent::changeListenerCallback(ChangeBroadcaster* source) 
{
	String msg("");

	if (transportSource.isPlaying())
	{
		sendMessage("file is playing");
	}
	else
	{
		sendMessage("file is stopped");
	}
}


void  AudioComponent::sendMessage(const char *p_str)
{
	String msg(p_str);
	event_queue.push(msg);
}


void AudioComponent::setPosition(float pos)
{
	transportSource.setPosition(pos);
}

void AudioComponent::stop()
{
	transportSource.stop();
	transportSource.setPosition(0.0);
}

int AudioComponent::setPlayFile(File &file)
{
	AudioFormatReader* reader = formatManager.createReaderFor(file); // [10]

	if (reader != nullptr)
	{
		ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(reader, true);
		transportSource.setSource(newSource, 0, nullptr, reader->sampleRate);
		readerSource = newSource.release();
		sendMessage("file loaded");
		return 0;
	}
	return 1;
}

void  AudioComponent::startToPlay()
{
	transportSource.start();
}


void AudioComponent::getEvent(String& event_msg)
{
	if ((!event_queue.empty()))
	{
		event_msg = event_queue.front();
		event_queue.pop();
	}
	else
	{
		event_msg = "";
	}

}

void AudioComponent::run()
{
	while (!threadShouldExit())
	{
		if ((!event_queue.empty()))
		{
			sendChangeMessage();
		}
		wait(500);
	}
}

AudioComponent *AudioComponentObj;
AudioComponent* createAudioComponent()
{
	AudioComponentObj = new AudioComponent();
	return AudioComponentObj;
}

extern "C" {
	void startToPlayFile(int argc, char * const argv[])
	{
		int i = 1;
		String filename(argv[0]);

		while (i < argc)
		{
			filename += " ";
			filename += argv[i++];
		}

		File file(filename);

		AudioComponentObj->setPlayFile(file);

	}

	void stopPlay()
	{
		AudioComponentObj->stop();
	}

	void startPlay()
	{
		AudioComponentObj->startToPlay();
	}

}
