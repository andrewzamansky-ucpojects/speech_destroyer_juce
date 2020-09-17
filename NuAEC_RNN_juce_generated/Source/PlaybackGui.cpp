#include "PlaybackGui.h"
#include "MainComponent.h"


PlaybackGui::PlaybackGui()
{
	setText("playback");
	addAndMakeVisible(playback_file_name_label);
	playback_file_name_label.setText(
			"no playback file added", juce::dontSendNotification );

	addAndMakeVisible (&playButton);
	playButton.setButtonText ("Play");
	playButton.onClick = [this] { playButtonClicked(); };
	playButton.setColour(
			juce::TextButton::buttonColourId, juce::Colours::green);
	playButton.setEnabled (false);

	addAndMakeVisible (&stopButton);
	stopButton.setButtonText ("Stop");
	stopButton.onClick = [this] { stopButtonClicked(); };
	stopButton.setColour (juce::TextButton::buttonColourId, juce::Colours::red);
	stopButton.setEnabled (false);


}

PlaybackGui::~PlaybackGui()
{
}

/*
void PlaybackGui::paint (juce::Graphics& g)
{
}
*/


#define BORDER_WIDTH  7
void PlaybackGui::resized()
{
	auto ctl_rect = getLocalBounds();
	int ctl_win_width;
	int ctl_win_height;

	ctl_rect.reduce(BORDER_WIDTH, BORDER_WIDTH);
	ctl_rect.removeFromTop(4);

	ctl_win_height = ctl_rect.getHeight();
	ctl_win_width = ctl_rect.getWidth();

	playback_file_name_label.setBounds(
			ctl_rect.removeFromTop(ctl_win_height / num_of_control_rows));
	auto playback_ctl_rect =
			ctl_rect.removeFromTop(ctl_win_height / num_of_control_rows);
	playButton.setBounds(playback_ctl_rect.removeFromLeft(ctl_win_width / 2));
	stopButton.setBounds(playback_ctl_rect);
}


void PlaybackGui::set_filename_str(const String &file_name_str)
{
	playback_file_name_label.setText(file_name_str, juce::dontSendNotification);
}


void PlaybackGui::set_main_component(MainComponent *new_mainComponent)
{
	mainComponent = new_mainComponent;
}


void PlaybackGui::changeState(enum playback_transport_state_e new_state)
{
	if (state != new_state)
	{
		state = new_state;

		switch (state)
		{
			case PLAYBACK_STOPPED:
				stopButton.setEnabled(false);
				playButton.setEnabled(true);
				break;

			case PLAYBACK_STARTING:
				playButton.setEnabled(false);
				break;

			case PLAYBACK_PLAYING:
				stopButton.setEnabled(true);
				break;

			case PLAYBACK_STOPPING:
				break;
		}
	}
}


void PlaybackGui::playButtonClicked()
{
	mainComponent->change_playback_state(PLAYBACK_STARTING);
}

void PlaybackGui::stopButtonClicked()
{
	mainComponent->change_playback_state(PLAYBACK_STOPPING);
}
