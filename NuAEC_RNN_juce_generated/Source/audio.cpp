#include "MainComponent.h"


void MainComponent::init_audio()
{
	String file_name(
		"C:\\Work\\audio_files\\"
		"Eagles_-_Hotel_California_Lyrics[ListenVid.com].wav");
	File audio_file(file_name);

    auto* reader = formatManager.createReaderFor(audio_file);

    if (reader != nullptr)
    {
    	playback_file_name_label.setText(file_name, juce::dontSendNotification);
        std::unique_ptr<juce::AudioFormatReaderSource> newSource(
        				new juce::AudioFormatReaderSource (reader, true));
        transportSource.setSource(
        		newSource.get(), 0, nullptr, reader->sampleRate);
        playButton.setEnabled(true);
        readerSource.reset(newSource.release());
    }
    else
    {
        playback_file_name_label.setText(
        		"no playback file added", juce::dontSendNotification );
    }
}

void MainComponent::prepareToPlay(
		int samplesPerBlockExpected, double sampleRate)
{
	transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock(
		const juce::AudioSourceChannelInfo& bufferToFill)
{
    if (readerSource.get() == nullptr)
    {
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transportSource.getNextAudioBlock (bufferToFill);

}


void MainComponent::changeState (TransportState newState)
{
    if (state != newState)
    {
        state = newState;

        switch (state)
        {
            case Stopped:                           // [3]
                stopButton.setEnabled (false);
                playButton.setEnabled (true);
                transportSource.setPosition (0.0);
                break;

            case Starting:                          // [4]
                playButton.setEnabled (false);
                transportSource.start();
                break;

            case Playing:                           // [5]
                stopButton.setEnabled (true);
                break;

            case Stopping:                          // [6]
                transportSource.stop();
                break;
        }
    }
}


void MainComponent::playback_transport_changed()
{
    if (transportSource.isPlaying())
        changeState (Playing);
    else
        changeState (Stopped);
}


void MainComponent::enProcessButtonClicked ()
{
	if ( true  == (enable_process_btn.getToggleState()))
	{
		enable_audio_process = 1;
	}
	else
	{
		enable_audio_process = 0;
	}
}

void MainComponent::playButtonClicked()
{
	changeState (Starting);
}

void MainComponent::stopButtonClicked()
{
	changeState (Stopping);
}
