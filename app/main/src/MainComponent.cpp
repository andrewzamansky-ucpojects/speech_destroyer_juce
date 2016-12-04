#ifndef MAINCOMPONENT_H_INCLUDED
#define MAINCOMPONENT_H_INCLUDED

#include "../JuceLibraryCode/JuceHeader.h"
#include "AudioComponent.h"

AudioComponent* createAudioComponent();


class MainContentComponent : public AudioAppComponent,
	public ChangeListener,
	public Button::Listener
{

public:
	MainContentComponent()
		: state(Stopped)
	{


		levelSlider.setRange(0.0, 0.25);
		levelSlider.setTextBoxStyle(Slider::TextBoxRight, false, 100, 20);
		levelLabel.setText("VB Level", dontSendNotification);

		addAndMakeVisible(levelSlider);
		addAndMakeVisible(levelLabel);

		addAndMakeVisible(&openButton);
		openButton.setButtonText("Open...");
		openButton.addListener(this);

		addAndMakeVisible(&playButton);
		playButton.setButtonText("Play");
		playButton.addListener(this);
		playButton.setColour(TextButton::buttonColourId, Colours::green);
		playButton.setEnabled(false);

		addAndMakeVisible(&stopButton);
		stopButton.setButtonText("Stop");
		stopButton.addListener(this);
		stopButton.setColour(TextButton::buttonColourId, Colours::red);
		stopButton.setEnabled(false);

		setSize(300, 200);

		AudioComponentObj = createAudioComponent();
		pControl_PC_App_Component_Obj = AudioComponentObj;
		AudioComponentObj->addChangeListener(this);
		// audio 
		setAudioChannels(0, 2);
	}

	/*  start of code needed for audio*/
	~MainContentComponent()
	{
		shutdownAudio();
		delete AudioComponentObj;
	}

	void prepareToPlay(int samplesPerBlockExpected, double sampleRate) override
	{
		AudioComponentObj->prepareToPlay(samplesPerBlockExpected, sampleRate);
	}


	void getNextAudioBlock(const AudioSourceChannelInfo& bufferToFill) override
	{
		AudioComponentObj->getNextAudioBlock(bufferToFill);
	}

	void releaseResources() override
	{
		AudioComponentObj->releaseResources();
	}
	/*  end of code needed for audio*/



	void changeListenerCallback(ChangeBroadcaster* source) override
	{
		if (source == AudioComponentObj)
		{
			String event_msg;
			AudioComponentObj->getEvent(event_msg);
			if (0 == event_msg.compare("file loaded"))
			{
				playButton.setEnabled(true);
			}
			else if (0 == event_msg.compare("file is playing"))
			{
				changeState(Playing);
			}
			else if (0 == event_msg.compare("file is stopped"))
			{
				changeState(Stopped);
			}
		}
	}


	void resized() override
	{
		openButton.setBounds(10, 10, getWidth() - 20, 20);
		playButton.setBounds(10, 40, getWidth() - 20, 20);
		stopButton.setBounds(10, 70, getWidth() - 20, 20);
		levelLabel.setBounds(10, 100, 90, 20);
		levelSlider.setBounds(100, 100, getWidth() - 110, 20);
	}

	void buttonClicked(Button* button) override
	{
		if (button == &openButton)  openButtonClicked();
		if (button == &playButton)  playButtonClicked();
		if (button == &stopButton)  stopButtonClicked();
	}

private:
	enum TransportState
	{
		Stopped,
		Starting,
		Playing,
		Stopping
	};

	void changeState(TransportState newState)
	{
		if (state != newState)
		{
			state = newState;

			switch (state)
			{
			case Stopped:                           // [3]
				stopButton.setEnabled(false);
				playButton.setEnabled(true);
				break;

			case Starting:                          // [4]
				playButton.setEnabled(false);
				pControl_PC_App_Component_Obj->sendCommand("start_play");

				break;

			case Playing:                           // [5]
				stopButton.setEnabled(true);
				break;

			case Stopping:                          // [6]
				pControl_PC_App_Component_Obj->sendCommand("stop_play");
				break;
			}
		}
	}

	void openButtonClicked()
	{
		FileChooser chooser("Select a Wave file to play...",
			File::nonexistent,
			"*.wav");                                        // [7]

		if (chooser.browseForFileToOpen())                                    // [8]
		{
			String file_name;
			String cmd("play_file ");
			const char *row_cmd;
			File file(chooser.getResult());
			file_name = file.getFullPathName();
			file_name = file_name.replace("\\", "\\\\");
			cmd += file_name; //append
			row_cmd = cmd.toRawUTF8();
			pControl_PC_App_Component_Obj->sendCommand(row_cmd);

		}
	}

	void playButtonClicked()
	{
		changeState(Starting);
	}

	void stopButtonClicked()
	{
		changeState(Stopping);
	}

	//==========================================================================
	TextButton openButton;
	TextButton playButton;
	TextButton stopButton;
	Random random;
	Slider levelSlider;
	Label levelLabel;

	TransportState state;
	AudioComponent *AudioComponentObj;
	ControlBaseComponent *pControl_PC_App_Component_Obj;

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainContentComponent)
};

Component* createMainContentComponent()
{
	return new MainContentComponent();
}


#endif  // MAINCOMPONENT_H_INCLUDED
