#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent(): audioSetupComp (deviceManager,
		  0,     // minimum input channels
		  256,   // maximum input channels
		  0,     // minimum output channels
		  256,   // maximum output channels
		  false, // ability to select midi inputs
		  false, // ability to select midi output device
		  false, // treat channels as stereo pairs
		  false) // hide advanced options

{
	using juce_run_permissions = juce::RuntimePermissions;

	addAndMakeVisible(audioSetupComp);

    addAndMakeVisible(enable_process_btn);
    enable_process_btn.setButtonText("process audio");
    enable_process_btn.onClick = [this] { enProcessButtonClicked(); };

    addAndMakeVisible(playback_file_name_label);

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


	addAndMakeVisible (diagnosticsBox);

	diagnosticsBox.setMultiLine (true);
	diagnosticsBox.setReturnKeyStartsNewLine (true);
	diagnosticsBox.setReadOnly (true);
	diagnosticsBox.setScrollbarsShown (true);
	diagnosticsBox.setCaretVisible (false);
	diagnosticsBox.setPopupMenuEnabled (true);
	diagnosticsBox.setColour(
			juce::TextEditor::backgroundColourId, juce::Colour (0x32ffffff));
	diagnosticsBox.setColour(
			juce::TextEditor::outlineColourId, juce::Colour (0x1c000000));
	diagnosticsBox.setColour(
			juce::TextEditor::shadowColourId, juce::Colour (0x16000000));

	cpuUsageLabel.setText ("CPU Usage", juce::dontSendNotification);
	cpuUsageText.setJustificationType (juce::Justification::right);
	addAndMakeVisible (&cpuUsageLabel);
	addAndMakeVisible (&cpuUsageText);


	// Some platforms require permissions to
	// open input channels so request that here
	if (juce_run_permissions::isRequired(juce_run_permissions::recordAudio)
		&& !juce_run_permissions::isGranted(juce_run_permissions::recordAudio))
	{
		juce_run_permissions::request(juce_run_permissions::recordAudio,
				[&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
	}
	else
	{
		// Specify the number of input and output channels that we want to open
		setAudioChannels (2, 2);
	}

	formatManager.registerBasicFormats();
	transportSource.addChangeListener(this);

	deviceManager.addChangeListener (this);
	init_audio();
	startTimer (50);
}

MainComponent::~MainComponent()
{
	// This shuts down the audio device and clears the audio source.
	deviceManager.removeChangeListener (this);
	shutdownAudio();
}


void MainComponent::releaseResources()
{
	transportSource.releaseResources();
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
	// (Our component is opaque, so we must completely fill the
	// background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(
				juce::ResizableWindow::backgroundColourId));
	g.setColour (juce::Colours::grey);
	g.fillRect (getLocalBounds().removeFromRight (proportionOfWidth (0.4f)));

	// You can add your drawing code here!
}

#define AUDIO_COMP_HEIGHT  320

void MainComponent::resized()
{
	auto rect = getLocalBounds();
	int num_of_control;
	int ctl_win_width;
	int ctl_win_height;

	num_of_control = 4;

	auto ctl_rect = rect.removeFromLeft(proportionOfWidth (0.6f));
	auto audio_setup_rect = ctl_rect.removeFromTop(AUDIO_COMP_HEIGHT);
	audioSetupComp.setBounds (audio_setup_rect);
	ctl_win_height = ctl_rect.getHeight();
	ctl_win_width = ctl_rect.getWidth();
	enable_process_btn.setBounds(
			ctl_rect.removeFromTop(ctl_win_height / num_of_control));
	playback_file_name_label.setBounds(
			ctl_rect.removeFromTop(ctl_win_height / num_of_control));
	auto playback_ctl_rect =
			ctl_rect.removeFromTop(ctl_win_height / num_of_control);
	playButton.setBounds(playback_ctl_rect.removeFromLeft(ctl_win_width / 2));
	stopButton.setBounds(playback_ctl_rect);

	rect.reduce (10, 10);
	auto topLine (rect.removeFromTop (20));
	cpuUsageLabel.setBounds (topLine.removeFromLeft (topLine.getWidth() / 2));
	cpuUsageText .setBounds (topLine);
	rect.removeFromTop (20);

	diagnosticsBox.setBounds (rect);
}



void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source)
{
    if (source == &transportSource)
    {
    	playback_transport_changed();
    }
    else if (source == &deviceManager)
    {
        dumpDeviceInfo();
    }

}


void MainComponent::timerCallback()
{
    auto cpu = deviceManager.getCpuUsage() * 100;
    cpuUsageText.setText(
    		juce::String (cpu, 6) + " %", juce::dontSendNotification);
}

void MainComponent::dumpDeviceInfo()
{
    logMessage ("--------------------------------------");
    logMessage ("Current audio device type: " +
    	(deviceManager.getCurrentDeviceTypeObject() != nullptr
           ? deviceManager.getCurrentDeviceTypeObject()->getTypeName()
                        : "<none>"));

    if (auto* device = deviceManager.getCurrentAudioDevice())
    {
        logMessage ("Current audio device: "   + device->getName().quoted());
        logMessage ("Sample rate: " +
        		juce::String (device->getCurrentSampleRate()) + " Hz");
        logMessage ("Block size: " +
        	juce::String (device->getCurrentBufferSizeSamples()) + " samples");
        logMessage("Bit depth: " + juce::String (device->getCurrentBitDepth()));
        logMessage ("Input channel names: " +
        		device->getInputChannelNames().joinIntoString (", "));
        logMessage ("Active input channels: " +
        		getListOfActiveBits (device->getActiveInputChannels()));
        logMessage ("Output channel names: "   +
        		device->getOutputChannelNames().joinIntoString (", "));
        logMessage ("Active output channels: " +
        		getListOfActiveBits (device->getActiveOutputChannels()));
    }
    else
    {
        logMessage ("No audio device open");
    }
}

void MainComponent::logMessage (const juce::String& m)
{
    diagnosticsBox.moveCaretToEnd();
    diagnosticsBox.insertTextAtCaret (m + juce::newLine);
}
