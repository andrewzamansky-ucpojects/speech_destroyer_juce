#pragma once

#include <JuceHeader.h>
#include "PlaybackGui.h"

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
	void changeState(enum playback_transport_state_e new_state);


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

	//==========================================================================
	juce::Random random;
	juce::AudioDeviceSelectorComponent audioSetupComp;
	juce::Label cpuUsageLabel;
	juce::Label cpuUsageText;
	juce::TextEditor diagnosticsBox;
	juce::ToggleButton enable_process_btn;
	PlaybackGui playbackGui;

	uint8_t enable_audio_process = 0;
	juce::AudioFormatManager formatManager;
	std::unique_ptr<juce::AudioFormatReaderSource> readerSource;
	juce::AudioTransportSource transportSource;
	enum playback_transport_state_e state;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
