#pragma once

#include <JuceHeader.h>
#include "PlaybackGui.h"
#include "RecordGui.h"

//==============================================================================
/*
   This component lives inside our window, and this is where you should put all
   your controls and content.
*/
class MainComponent  : public juce::AudioAppComponent,
						public juce::ChangeListener,
						private juce::Timer
{
public:
	//=========================================================================
	MainComponent();

	~MainComponent() override;


	//=========================================================================
	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override;
	void getNextAudioBlock(
			const juce::AudioSourceChannelInfo& bufferToFill) override;
	void releaseResources() override;

	//==========================================================================
	void paint (juce::Graphics& g) override;

	void resized() override;
	void change_playback_state(enum playback_transport_state_e new_state);
	void change_recording_state(
			enum record_transport_state_e new_state, int duration_ms);


private:

	static uint8_t num_of_objects;

	void changeListenerCallback (juce::ChangeBroadcaster*) override;

	static juce::String getListOfActiveBits (const juce::BigInteger& b)
	{
		juce::StringArray bits;

		for (auto i = 0; i <= b.getHighestBit(); ++i)
			if (b[i])
				bits.add (juce::String (i));

		return bits.joinIntoString (", ");
	}
	void timerCallback() override;
	void dumpDeviceInfo();

	void logMessage (const juce::String& m);

	void enProcessButtonClicked();
	void playback_transport_changed();
	void init_audio();
	void free_audio();
	void init_playback();
	void init_recorder();
	void do_record(float *left_buff_to_record,
			float *right_buff_to_record, size_t samples_to_record);
	void process_audio(
			float **left_buff_to_record, float **right_buff_to_record,
			size_t *samples_to_record);
	void add_new_input_buffers(size_t num_of_samples,
			const float *new_buf_left, const float *new_buf_right,
			uint8_t updating_playback_buffers);
	uint8_t get_buffers_for_processing();
	void  init_audio_processing();
	int record_start();

	//==========================================================================
	juce::Random random;
	juce::AudioDeviceSelectorComponent audioSetupComp;
	juce::Label cpuUsageLabel;
	juce::Label cpuUsageText;
	juce::TextEditor diagnosticsBox;
	juce::ToggleButton enable_process_btn;
	PlaybackGui playbackGui;
	RecordGui recordGui;

	uint8_t enable_audio_process = 0;
	juce::AudioFormatManager formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource transportSource;
	enum playback_transport_state_e playback_state;
	enum record_transport_state_e record_state;
	int curr_rec_duration_ms = 0;
	int max_rec_duration_ms = 0;

	size_t in_audio_write_pointer = 0;
	size_t in_audio_read_pointer = 0;
	size_t valid_data_in_input_buffer = 0;

	// length should be (buff_size_of_DSP_chain * const)
	#define DSP_BUFF_SIZE  (3*512)
	#define CYCLIC_BUFF_SIZE  (DSP_BUFF_SIZE * 7)

	float mic_buff_left_cyclic[CYCLIC_BUFF_SIZE];
	float mic_buff_right_cyclic[CYCLIC_BUFF_SIZE];
	float playback_buff_left_cyclic[CYCLIC_BUFF_SIZE];
	float playback_buff_right_cyclic[CYCLIC_BUFF_SIZE];

	int record_sample_rate = 16000;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
