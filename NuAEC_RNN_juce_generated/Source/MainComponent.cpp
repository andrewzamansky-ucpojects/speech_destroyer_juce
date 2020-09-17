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

	setSize (600, 400); // initial size

	addAndMakeVisible(audioSetupComp);
	addAndMakeVisible(playbackGui);
	addAndMakeVisible(recordGui);

	addAndMakeVisible(enable_process_btn);
	enable_process_btn.setButtonText("process audio in AEC+RNNoise");
	enable_process_btn.onClick = [this] { enProcessButtonClicked(); };

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
	free_audio();
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
	int num_of_control_rows;
	int ctl_win_height;
	int control_height;

	num_of_control_rows = 1 + playbackGui.num_of_control_rows +
			 + recordGui.num_of_control_rows;

	auto ctl_rect = rect.removeFromLeft(proportionOfWidth (0.6f));
	auto audio_setup_rect = ctl_rect.removeFromTop(AUDIO_COMP_HEIGHT);
	audioSetupComp.setBounds (audio_setup_rect);
	ctl_win_height = ctl_rect.getHeight();

	control_height = (1 * ctl_win_height) / num_of_control_rows;
	enable_process_btn.setBounds(ctl_rect.removeFromTop(control_height));

	control_height = (playbackGui.num_of_control_rows * ctl_win_height) /
														num_of_control_rows;
	playbackGui.setBounds(ctl_rect.removeFromTop(control_height));

	control_height = (recordGui.num_of_control_rows * ctl_win_height) /
														num_of_control_rows;
	recordGui.setBounds(ctl_rect.removeFromTop(control_height));

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

#define SMOOTHING_ALPHA_RAISING  0.1f
#define SMOOTHING_ALPHA_FALLING  0.01f
static double smoothed_cpu_usage = 0.0f;

void MainComponent::timerCallback()
{
	double alpha;
	double cpu = deviceManager.getCpuUsage() * 100;

	if (smoothed_cpu_usage < cpu)
	{
		alpha = SMOOTHING_ALPHA_RAISING;
	}
	else
	{
		alpha = SMOOTHING_ALPHA_FALLING;
	}

	smoothed_cpu_usage = (alpha * cpu) + ((1.0f - alpha) * smoothed_cpu_usage);
	cpuUsageText.setText(
		juce::String(smoothed_cpu_usage, 6) + " %", juce::dontSendNotification);

	recordGui.update_rec_elapsed(curr_rec_duration_ms);

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
