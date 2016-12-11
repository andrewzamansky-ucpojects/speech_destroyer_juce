#ifndef AUDIOCOMPONENT_H_INCLUDED
#define AUDIOCOMPONENT_H_INCLUDED

#include <queue>          // std::queue

#include "../JuceLibraryCode/JuceHeader.h"
extern "C" {
	#include "memory_pool_api.h"
	#include "dev_management_api.h"
	#include "dsp_management_api.h"
	#include "mixer_api.h"
	#include "mixer2x1_api.h"
	#include "webrtc_voice_activity_detection_api.h"

	void mixer2x1_init();
	void webrtc_voice_activity_detection_init();
	extern int run_command(const char *cmd, int flag);

}

#include "ControlBaseComponent.h"

class Control_PC_App_Component;//forward declaration

class AudioComponent : public AudioSource,
	public ChangeListener,
	public ControlBaseComponent,
	private Thread
{
private :
	void *dsp_buffers_pool;
	dsp_chain_t *pMain_dsp_chain;

	dsp_descriptor_t vad;
	dsp_descriptor_t stereo_to_mono;


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
	void getEvent(String& event) override;


private:


	//==========================================================================
	std::queue<String> event_queue;


	int bufferSize = 512;
	int currNumOfWordsInIntermediateBuffer;
	AudioSourceChannelInfo *localBufferToFill;
	AudioSampleBuffer   	*bufferToUse;

	Random random;
	int(*callback)(int);
	AudioFormatManager formatManager;
	ScopedPointer<AudioFormatReaderSource> readerSource;
	AudioTransportSource transportSource;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioComponent)
};


#endif  // AUDIOCOMPONENT_H_INCLUDED
