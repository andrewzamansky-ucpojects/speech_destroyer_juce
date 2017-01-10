
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

	auto_init_api();

	addControlCommands();

	app_dev_create_signal_flow();

	/**************   LPF path  *************/
	ch_weight.weight = 0.5;
	ch_weight.channel_num = 0;
	DSP_IOCTL_1_PARAMS(&stereo_to_mono, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	ch_weight.channel_num = 1;
	DSP_IOCTL_1_PARAMS(&stereo_to_mono, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	//			dsp_management_api_set_module_control(&stereo_to_mono , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

	currNumOfWordsInIntermediateBuffer = 0;

	bufferToUse = new AudioSampleBuffer(2, bufferSize);
	localBufferToFill = new AudioSourceChannelInfo(bufferToUse, 0, bufferSize);




	formatManager.registerBasicFormats();       // [1]
	transportSource.addChangeListener(this);   // [2]

	startThread();
}

AudioComponent::~AudioComponent()
{
	stopThread(1000);
	delete localBufferToFill;
	delete bufferToUse;
	DSP_DELETE_CHAIN(pMain_dsp_chain);
	memory_pool_delete(dsp_buffers_pool);
	deleteControlCommands();
	DSP_DELETE_MODULES();
}

void AudioComponent::prepareToPlay(int samplesPerBlockExpected, double sampleRate) 
{
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}


void AudioComponent::getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) 
{
	float* bufferToFillLeft;
	float* bufferToFillRight;
	float *intermediateBufferOfConstantSizeLeft;
	float *intermediateBufferOfConstantSizeRight;
	int bufferToFillSize;
	int wordsToCopy;

	if (readerSource == nullptr)
	{
		bufferToFill.clearActiveBufferRegion();
		return;
	}

	bufferToFillSize = bufferToFill.numSamples;
	bufferToFillLeft = bufferToFill.buffer->getWritePointer(0, bufferToFill.startSample);
	bufferToFillRight = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);

	intermediateBufferOfConstantSizeLeft = localBufferToFill->buffer->getWritePointer(0, 0);
	intermediateBufferOfConstantSizeRight = localBufferToFill->buffer->getWritePointer(0, 1);

	while ( bufferToFillSize)
	{

		if (0 == currNumOfWordsInIntermediateBuffer)
		{
			float *buffer_out1;
			float *buffer_out2;

			transportSource.getNextAudioBlock(*localBufferToFill);

			buffer_out1 = (float *)malloc(bufferSize*sizeof(float));
			buffer_out2 = (float *)malloc(bufferSize*sizeof(float));
			DSP_SET_CHAIN_INPUT_BUFFER(pMain_dsp_chain, DSP_INPUT_PAD_0, (float*)intermediateBufferOfConstantSizeLeft);
			DSP_SET_CHAIN_INPUT_BUFFER(pMain_dsp_chain, DSP_INPUT_PAD_1, (float*)intermediateBufferOfConstantSizeRight);
			DSP_SET_CHAIN_OUTPUT_BUFFER(pMain_dsp_chain, DSP_OUTPUT_PAD_0, (float*)buffer_out1);
			DSP_SET_CHAIN_OUTPUT_BUFFER(pMain_dsp_chain, DSP_OUTPUT_PAD_1, (float*)buffer_out2);
			DSP_PROCESS_CHAIN(pMain_dsp_chain, bufferSize);
			memcpy(intermediateBufferOfConstantSizeLeft  , buffer_out1, bufferSize * sizeof(float));
			memcpy(intermediateBufferOfConstantSizeRight , buffer_out2, bufferSize * sizeof(float));
			free(buffer_out1);
			free(buffer_out2);
			currNumOfWordsInIntermediateBuffer = bufferSize;
		}

		if (currNumOfWordsInIntermediateBuffer < bufferToFillSize)
		{
			wordsToCopy = currNumOfWordsInIntermediateBuffer;
		}
		else
		{
			wordsToCopy = bufferToFillSize;
		}
		memcpy(bufferToFillLeft, &intermediateBufferOfConstantSizeLeft[bufferSize - currNumOfWordsInIntermediateBuffer], wordsToCopy * sizeof(float));
		memcpy(bufferToFillRight, &intermediateBufferOfConstantSizeRight[bufferSize - currNumOfWordsInIntermediateBuffer], wordsToCopy * sizeof(float));
		bufferToFillLeft += wordsToCopy;
		bufferToFillRight += wordsToCopy;
		bufferToFillSize -= wordsToCopy;
		currNumOfWordsInIntermediateBuffer -= wordsToCopy;
	}
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
		msg="file is playing";
	}
	else
	{
		msg="file is stopped";
	}
	sendMessage(msg);
}


void  AudioComponent::sendMessage(String &msg)
{
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
		String msg("file loaded");
		ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource(reader, true);
		transportSource.setSource(newSource, 0, nullptr, reader->sampleRate);
		readerSource = newSource.release();
		sendMessage(msg);
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
		else
		{
			String msg("vad_result ");
			float vad_result;
			DSP_IOCTL_1_PARAMS(&vad , IOCTL_WEBRTC_VOICE_ACTIVITY_DETECTION_GET_RESULT , &vad_result );
			msg +=String(vad_result);
			sendMessage(msg);

		}
		wait(100);
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
