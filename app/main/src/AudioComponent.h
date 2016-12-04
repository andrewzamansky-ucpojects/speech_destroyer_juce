#ifndef AUDIOCOMPONENT_H_INCLUDED
#define AUDIOCOMPONENT_H_INCLUDED

#include <queue>          // std::queue

#include "../JuceLibraryCode/JuceHeader.h"
extern "C" {
	#include "memory_pool_api.h"
	#include "dev_management_api.h"
	#include "dsp_management_api.h"
	#include "biquad_filter_api.h"
	#include "mixer_api.h"
	#include "mixer2x1_api.h"
	#include "mixer3x1_api.h"
	#include "lookahead_compressor_api.h"
	#include "standard_compressor_api.h"
	#include "virtual_bass_api.h"
	#include "voice_3D_api.h"

	void mixer3x1_init();
	void mixer2x1_init();
	void voice_3D_init();
	void  biquad_filter_init();
	void standard_compressor_init();
	void lookahead_compressor_init();
	void virtual_bass_init();
	extern int run_command(const char *cmd, int flag);

}

#include "ControlBaseComponent.h"

class Control_PC_App_Component;//forward declaration

class AudioComponent : public AudioSource,
	public ChangeListener,
	public ChangeBroadcaster,
	public ControlBaseComponent,
	private Thread
{
private :
	void *dsp_buffers_pool;
	dsp_chain_t *pMain_dsp_chain;

	dsp_descriptor_t lpf_filter;

	dsp_descriptor_t vb_final_filter;


	dsp_descriptor_t hpf_filter_left;
	dsp_descriptor_t hpf_filter_right;

	dsp_descriptor_t mpf_filter_left;
	dsp_descriptor_t mpf_filter_right;

	dsp_descriptor_t hpf_voice_3d;
	dsp_descriptor_t mpf_voice_3d;

	dsp_descriptor_t compressor_hf;
	dsp_descriptor_t compressor_mf;

	dsp_descriptor_t leftChanelEQ;
	dsp_descriptor_t rightChanelEQ;

	dsp_descriptor_t limiter;

	dsp_descriptor_t stereo_to_mono;
	dsp_descriptor_t adder_bass_with_left_channel;
	dsp_descriptor_t adder_bass_with_right_channel;

	float cutoff_freq_low = 100;
	float cutoff_freq_high = 4000;

	void sendMessage(const char *p_str);

public:
	AudioComponent();

	~AudioComponent();

	uint8_t app_dev_create_signal_flow();
	uint8_t app_dev_set_cuttof();

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;
	void changeListenerCallback(ChangeBroadcaster* source) override;
	void run() override;
	//void setStateCallback(ChangeListener callback);
	int setPlayFile(File &file);
	void startToPlay();
	int isPlaying();
	void setPosition(float pos);
	void stop();
	void sendCommand(const char* cmd) override;
	void addControlCommands();
	void getEvent(String& event);


private:


	//==========================================================================
	std::queue<String> event_queue;

	Random random;
	int(*callback)(int);
	AudioFormatManager formatManager;
	ScopedPointer<AudioFormatReaderSource> readerSource;
	AudioTransportSource transportSource;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioComponent)
};


#endif  // AUDIOCOMPONENT_H_INCLUDED
