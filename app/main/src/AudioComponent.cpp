
#include "AudioComponent.h"
#include <queue>          // std::queue

extern "C" {

	dsp_descriptor_t vb;

}
#if 0
void AudioComponent::setStateCallback(ChangeListener callback)
{
	//callback = new_callback;
}
#endif

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
	/*
	-> hpf_filter_left  ->                              ------>     => adder_bass_with_left_channel -> leftChanelEQ --
	/                       \                            /          /                                                     \
	/                          => hpf_voice_3d   ===========>           /                                                        => limiter => app_I2S_mixer
	/                         /                            \        /                                                       /
	-> hpf_filter_right ->                                ------>/ => adder_bass_with_right_channel  -> rightChanelEQ  --
	/                                                               /
	source ->  app_I2S_spliter                                                                 /
	\                                                              /
	=> stereo_to_mono -> lpf_filter -> virtual_bass ---------> /

	*/

	pMain_dsp_chain = DSP_CREATE_CHAIN(18);
	/******** LF path modules**********/
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, MIXER2X1_API_MODULE_NAME, &stereo_to_mono);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &lpf_filter);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, VIRTUAL_BASS_API_MODULE_NAME, &vb);
	/********* end of LF path    **********/

	/******** HF path modules**********/
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &hpf_filter_left);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &hpf_filter_right);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, VOICE_3D_API_MODULE_NAME, &hpf_voice_3d);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, STANDARD_COMPRESSOR_API_MODULE_NAME, &compressor_hf);
	/********* end of HF path    **********/

	/******** MF path modules**********/
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &mpf_filter_left);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &mpf_filter_right);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, VOICE_3D_API_MODULE_NAME, &mpf_voice_3d);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, STANDARD_COMPRESSOR_API_MODULE_NAME, &compressor_mf);
	/********* end of MF path    **********/

	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, MIXER3X1_API_MODULE_NAME, &adder_bass_with_left_channel);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, MIXER3X1_API_MODULE_NAME, &adder_bass_with_right_channel);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &leftChanelEQ);
	DSP_ADD_MODULE_TO_CHAIN(pMain_dsp_chain, BIQUAD_FILTER_API_MODULE_NAME, &rightChanelEQ);



	/**************** module links  **********************/
	/******** LF path **********/
	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain, DSP_INPUT_PAD_0, &stereo_to_mono, DSP_INPUT_PAD_0);
	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain, DSP_INPUT_PAD_0, &stereo_to_mono, DSP_INPUT_PAD_1);

	DSP_CREATE_INTER_MODULES_LINK(&stereo_to_mono, DSP_OUTPUT_PAD_0, &lpf_filter, DSP_INPUT_PAD_0);

	/********  VB  ************/
	DSP_CREATE_INTER_MODULES_LINK(&lpf_filter, DSP_OUTPUT_PAD_0, &vb, DSP_INPUT_PAD_0);

	/******* end of VB   *********/

	/********* end of LF path    **********/


	/********** HF path *********/

	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain, DSP_INPUT_PAD_0, &hpf_filter_left, DSP_INPUT_PAD_0);
	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain, DSP_INPUT_PAD_0, &hpf_filter_right, DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&hpf_filter_left, DSP_OUTPUT_PAD_0, &hpf_voice_3d, DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&hpf_filter_right, DSP_OUTPUT_PAD_0, &hpf_voice_3d, DSP_INPUT_PAD_1);

	DSP_CREATE_INTER_MODULES_LINK(&hpf_voice_3d, DSP_OUTPUT_PAD_0, &compressor_hf, DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&hpf_voice_3d, DSP_OUTPUT_PAD_1, &compressor_hf, DSP_INPUT_PAD_1);

	/********* end of HF path  *********/

	/********** MF path *********/

	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain, DSP_INPUT_PAD_0, &mpf_filter_left, DSP_INPUT_PAD_0);
	DSP_CREATE_CHAIN_INPUT_TO_MODULE_LINK(pMain_dsp_chain, DSP_INPUT_PAD_0, &mpf_filter_right, DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&mpf_filter_left, DSP_OUTPUT_PAD_0, &mpf_voice_3d, DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&mpf_filter_right, DSP_OUTPUT_PAD_0, &mpf_voice_3d, DSP_INPUT_PAD_1);

	DSP_CREATE_INTER_MODULES_LINK(&mpf_voice_3d, DSP_OUTPUT_PAD_0, &compressor_mf, DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&mpf_voice_3d, DSP_OUTPUT_PAD_1, &compressor_mf, DSP_INPUT_PAD_1);

	/********* end of MF path  *********/

	/********** collecting LF ,MF and HF pathes together with phase change on HF *********/
	DSP_CREATE_INTER_MODULES_LINK(&vb, DSP_OUTPUT_PAD_0, &adder_bass_with_left_channel, DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&compressor_hf, DSP_OUTPUT_PAD_0, &adder_bass_with_left_channel, DSP_INPUT_PAD_1);
	DSP_CREATE_INTER_MODULES_LINK(&compressor_mf, DSP_OUTPUT_PAD_0, &adder_bass_with_left_channel, DSP_INPUT_PAD_2);

	DSP_CREATE_INTER_MODULES_LINK(&vb, DSP_OUTPUT_PAD_0, &adder_bass_with_right_channel, DSP_INPUT_PAD_0);
	DSP_CREATE_INTER_MODULES_LINK(&compressor_hf, DSP_OUTPUT_PAD_1, &adder_bass_with_right_channel, DSP_INPUT_PAD_1);
	DSP_CREATE_INTER_MODULES_LINK(&compressor_mf, DSP_OUTPUT_PAD_1, &adder_bass_with_right_channel, DSP_INPUT_PAD_2);

	/********** end of collecting LF ,MF and HF pathes together  *********/

	/********** EQ+Compressor  *********/

	DSP_CREATE_INTER_MODULES_LINK(&adder_bass_with_left_channel, DSP_OUTPUT_PAD_0, &leftChanelEQ, DSP_INPUT_PAD_0);

	DSP_CREATE_INTER_MODULES_LINK(&adder_bass_with_right_channel, DSP_OUTPUT_PAD_0, &rightChanelEQ, DSP_INPUT_PAD_0);

	DSP_CREATE_MODULE_TO_CHAIN_OUTPUT_LINK(pMain_dsp_chain, DSP_OUTPUT_PAD_0, &leftChanelEQ, DSP_OUTPUT_PAD_0);
	DSP_CREATE_MODULE_TO_CHAIN_OUTPUT_LINK(pMain_dsp_chain, DSP_OUTPUT_PAD_1, &rightChanelEQ, DSP_OUTPUT_PAD_0);

	return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
/* Function:        app_dev_set_cuttof                                                                          */
/*                                                                                                         */
/* Parameters:                                                                                             */
/*                                                                                         */
/*                                                                                                  */
/* Returns:                                                                                      */
/* Side effects:                                                                                           */
/* Description:                                                                                            */
/*                                                            						 */
/*---------------------------------------------------------------------------------------------------------*/
uint8_t AudioComponent::app_dev_set_cuttof()
{
	biquad_filter_api_band_set_t band_set;
	biquad_filter_api_band_set_params_t  *p_band_set_params;
	float freq;

	p_band_set_params = &band_set.band_set_params;

	p_band_set_params->Gain = 1;

	p_band_set_params->Fc = cutoff_freq_low;
	p_band_set_params->QValue = 1;//0.836;//0.707;
	p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_1_POLE;
	band_set.band_num = 0;
	DSP_IOCTL_1_PARAMS(&lpf_filter, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);

	p_band_set_params->Fc = cutoff_freq_low;
	p_band_set_params->QValue = 1;//0.836;//0.707;
	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	band_set.band_num = 0;
	DSP_IOCTL_1_PARAMS(&mpf_filter_left, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	DSP_IOCTL_1_PARAMS(&mpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
	band_set.band_num = 1;
	DSP_IOCTL_1_PARAMS(&mpf_filter_left, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	DSP_IOCTL_1_PARAMS(&mpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	p_band_set_params->Fc = cutoff_freq_high;
	p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
	p_band_set_params->QValue = 0.707;//0.836;//0.707;
	band_set.band_num = 2;
	DSP_IOCTL_1_PARAMS(&mpf_filter_left, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	DSP_IOCTL_1_PARAMS(&mpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
	band_set.band_num = 3;
	DSP_IOCTL_1_PARAMS(&mpf_filter_left, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	DSP_IOCTL_1_PARAMS(&mpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);


	p_band_set_params->Fc = cutoff_freq_high;
	p_band_set_params->QValue = 0.707;//0.836;//0.707;
	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	band_set.band_num = 0;
	DSP_IOCTL_1_PARAMS(&hpf_filter_left, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	DSP_IOCTL_1_PARAMS(&hpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	band_set.band_num = 1;
	DSP_IOCTL_1_PARAMS(&hpf_filter_left, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);
	DSP_IOCTL_1_PARAMS(&hpf_filter_right, IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set);

	freq = 100;
	DSP_IOCTL_1_PARAMS(&vb, IOCTL_VIRTUAL_BASS_SET_FIRST_HPF, &cutoff_freq_low);
	DSP_IOCTL_1_PARAMS(&vb, IOCTL_VIRTUAL_BASS_SET_SECOND_HPF, &freq);

	//	p_band_set_params->QValue = 1;//0.836;//0.707;
	//	p_band_set_params->Gain = 1;
	//	p_band_set_params->Fc = cutoff_freq_low * 3.4;
	//	p_band_set_params->filter_mode = BIQUADS_LOWPASS_MODE_2_POLES;
	//	band_set.band_num = 0;
	//	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	//	p_band_set_params->Fc = cutoff_freq_low * 0.2;
	//	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_1_POLE;
	//	band_set.band_num = 1;
	//	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );
	//	p_band_set_params->QValue = 1;//0.836;//0.707;
	//	p_band_set_params->filter_mode = BIQUADS_HIGHPASS_MODE_2_POLES;
	//	band_set.band_num = 2;
	//	DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_BIQUAD_FILTER_SET_BAND_BIQUADS, &band_set );


	return 0;
}

AudioComponent::AudioComponent() 
 : Thread("Background Thread")
{
	float threshold;
	float gain;
	set_channel_weight_t ch_weight;

	addControlCommands();

	mixer3x1_init();
	mixer2x1_init();
	voice_3D_init();
	biquad_filter_init();
	standard_compressor_init();
	lookahead_compressor_init();
	virtual_bass_init();



	dsp_buffers_pool = memory_pool_init(20, I2S_BUFF_LEN * sizeof(float));
	dsp_management_api_set_buffers_pool(dsp_buffers_pool);

	app_dev_create_signal_flow();

#if 1
	/**************   LPF path  *************/
	ch_weight.weight = 0.5;
	ch_weight.channel_num = 0;
	DSP_IOCTL_1_PARAMS(&stereo_to_mono, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	ch_weight.channel_num = 1;
	DSP_IOCTL_1_PARAMS(&stereo_to_mono, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	//			dsp_management_api_set_module_control(&stereo_to_mono , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

	DSP_IOCTL_1_PARAMS(&lpf_filter, IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS, 1);
	//			dsp_management_api_set_module_control(&lpf_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

	//dsp_management_api_set_module_control(&vb, DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

	//			DSP_IOCTL_1_PARAMS(&vb_final_filter , IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS , 3 );
	//			dsp_management_api_set_module_control(&vb_final_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_MUTE);
	//			dsp_management_api_set_module_control(&lpf_filter , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);


	/**************   HPF path  *************/
	DSP_IOCTL_1_PARAMS(&hpf_filter_left, IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS, 2);
	DSP_IOCTL_1_PARAMS(&hpf_filter_right, IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS, 2);
	//			dsp_management_api_set_module_control(&hpf_filter_left , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	//			dsp_management_api_set_module_control(&hpf_filter_right , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

	gain = 0.5;
	DSP_IOCTL_1_PARAMS(&hpf_voice_3d, IOCTL_VOICE_3D_SET_MEDIUM_GAIN, &gain);
	gain = 0.5;
	DSP_IOCTL_1_PARAMS(&hpf_voice_3d, IOCTL_VOICE_3D_SET_SIDE_GAIN, &gain);
	gain = 0;
	DSP_IOCTL_1_PARAMS(&hpf_voice_3d, IOCTL_VOICE_3D_SET_3D_GAIN, &gain);
	dsp_management_api_set_module_control(&hpf_voice_3d, DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	dsp_management_api_set_module_control(&compressor_hf, DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

	/**************   MPF path  *************/
	DSP_IOCTL_1_PARAMS(&mpf_filter_left, IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS, 4);
	DSP_IOCTL_1_PARAMS(&mpf_filter_right, IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS, 4);
	//dsp_management_api_set_module_control(&mpf_filter_left , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	//dsp_management_api_set_module_control(&mpf_filter_right , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

	gain = 0.5;
	DSP_IOCTL_1_PARAMS(&mpf_voice_3d, IOCTL_VOICE_3D_SET_MEDIUM_GAIN, &gain);
	gain = 0.5;
	DSP_IOCTL_1_PARAMS(&mpf_voice_3d, IOCTL_VOICE_3D_SET_SIDE_GAIN, &gain);
	gain = 0;
	DSP_IOCTL_1_PARAMS(&mpf_voice_3d, IOCTL_VOICE_3D_SET_3D_GAIN, &gain);
	dsp_management_api_set_module_control(&mpf_voice_3d, DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	dsp_management_api_set_module_control(&compressor_mf, DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);

	/**************  final mixed LPF+HPF path  *************/
	ch_weight.channel_num = 0;
	ch_weight.weight = 1;
	DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	ch_weight.channel_num = 1;
	ch_weight.weight = -1;
	DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	ch_weight.channel_num = 2;
	ch_weight.weight = -1;
	DSP_IOCTL_1_PARAMS(&adder_bass_with_left_channel, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	DSP_IOCTL_1_PARAMS(&adder_bass_with_right_channel, IOCTL_MIXER_SET_CHANNEL_WEIGHT, &ch_weight);
	//			dsp_management_api_set_module_control(&adder_bass_with_left_channel , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	//			dsp_management_api_set_module_control(&adder_bass_with_right_channel , DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);



	/*    eq filters   */
	DSP_IOCTL_1_PARAMS(&leftChanelEQ, IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS, 7);
	DSP_IOCTL_1_PARAMS(&rightChanelEQ, IOCTL_BIQUAD_FILTER_SET_NUM_OF_BANDS, 7);
	dsp_management_api_set_module_control(&leftChanelEQ, DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	dsp_management_api_set_module_control(&rightChanelEQ, DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
		 
	/*
	DSP_IOCTL_1_PARAMS(&limiter, IOCTL_LOOKAHEAD_COMPRESSOR_SET_TYPE, LOOKAHEAD_COMPRESSOR_API_TYPE_LIMITER);
	threshold = 0.9999;
	DSP_IOCTL_1_PARAMS(&limiter, IOCTL_LOOKAHEAD_COMPRESSOR_SET_HIGH_THRESHOLD, &threshold);
	DSP_IOCTL_1_PARAMS(&limiter, IOCTL_LOOKAHEAD_COMPRESSOR_SET_LOOK_AHEAD_SIZE, LATENCY_LENGTH);
	dsp_management_api_set_module_control(&limiter, DSP_MANAGEMENT_API_MODULE_CONTROL_BYPASS);
	*/
	app_dev_set_cuttof();
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
	float vb_volume;
	float *buffer_out1;
	float *buffer_out2;
	buff_len = bufferToFill.numSamples;
//	vb_volume = 200 * level;
	//DSP_IOCTL_1_PARAMS(&vb, IOCTL_VIRTUAL_BASS_SET_GAIN, &vb_volume);
	//run_command("set_vbi 30",0);
//	Control_PC_App_Component_Obj.sendCommand("set_vbi 30");

	buffer_out1 = (float *)malloc(buff_len*sizeof(float));
	buffer_out2 = (float *)malloc(buff_len*sizeof(float));
	DSP_SET_CHAIN_INPUT_BUFFER(pMain_dsp_chain, DSP_INPUT_PAD_0, (float*)buffer1);
	DSP_SET_CHAIN_INPUT_BUFFER(pMain_dsp_chain, DSP_INPUT_PAD_1, (float*)buffer2);
	DSP_SET_CHAIN_OUTPUT_BUFFER(pMain_dsp_chain, DSP_OUTPUT_PAD_0, (float*)buffer_out1);
	DSP_SET_CHAIN_OUTPUT_BUFFER(pMain_dsp_chain, DSP_OUTPUT_PAD_1, (float*)buffer_out2);
	DSP_PROCESS_CHAIN(pMain_dsp_chain, buff_len);
	memcpy(buffer1, buffer_out1, buff_len*sizeof(float));
	memcpy(buffer2, buffer_out2, buff_len*sizeof(float));
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
