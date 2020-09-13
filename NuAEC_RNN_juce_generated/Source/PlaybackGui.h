#pragma once

#include <JuceHeader.h>

enum playback_transport_state_e
{
	PLAYBACK_STOPPED,
	PLAYBACK_STARTING,
	PLAYBACK_PLAYING,
	PLAYBACK_STOPPING
};


class MainComponent;

class PlaybackGui : public GroupComponent
{
public:
	PlaybackGui();
	~PlaybackGui() override;

//	void paint (juce::Graphics& g) override;
	void resized() override;
	void set_filename_str(const juce::String &file_name_str);
	void set_main_component(MainComponent *new_mainComponent);
	void changeState(enum playback_transport_state_e new_state);
	int num_of_control_rows = 2;

private:
	juce::Label playback_file_name_label;
	juce::TextButton playButton;
	juce::TextButton stopButton;
	enum playback_transport_state_e state;
	MainComponent *mainComponent;

	void playButtonClicked();
	void stopButtonClicked();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PlaybackGui)
};
