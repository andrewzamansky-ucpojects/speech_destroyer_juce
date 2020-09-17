#include "RecordGui.h"
#include "MainComponent.h"


RecordGui::RecordGui()
{
	setText("record");
	addAndMakeVisible(record_file_name_label);
	record_file_name_label.setText(
			"no record file selected", juce::dontSendNotification );

	addAndMakeVisible(duration_title);
	duration_title.setText(
			"record duration (ms):", juce::dontSendNotification );
	duration_title.attachToComponent (&duration_input, true);
	duration_title.setColour (juce::Label::textColourId, juce::Colours::orange);
	duration_title.setJustificationType (juce::Justification::right);


    addAndMakeVisible (duration_input);
    duration_input.setEditable (true);
    duration_input.setColour(
    				juce::Label::backgroundColourId, juce::Colours::darkblue);
    duration_input.setText("10000", juce::dontSendNotification );

	addAndMakeVisible(duration_used);
	duration_used.setText("recorded - 0%", juce::dontSendNotification );

	addAndMakeVisible (&startRecordButton);
	startRecordButton.setButtonText ("Start Record");
	startRecordButton.onClick = [this] { startRecordButtonClicked(); };
	startRecordButton.setColour(
			juce::TextButton::buttonColourId, juce::Colours::green);
	startRecordButton.setEnabled (false);

	addAndMakeVisible (&stopRecordButton);
	stopRecordButton.setButtonText ("Stop");
	stopRecordButton.onClick = [this] { stopRecordButtonClicked(); };
	stopRecordButton.setColour(
			juce::TextButton::buttonColourId, juce::Colours::red);
	stopRecordButton.setEnabled (false);

	addAndMakeVisible (&deleteRecordButton);
	deleteRecordButton.setButtonText ("Delete record");
	deleteRecordButton.onClick = [this] { deleteRecordButtonClicked(); };
	deleteRecordButton.setColour(
			juce::TextButton::buttonColourId, juce::Colours::orange);
	deleteRecordButton.setEnabled (false);


}

RecordGui::~RecordGui()
{
}

/*
void RecordGui::paint (juce::Graphics& g)
{
}
*/


#define BORDER_WIDTH  7
void RecordGui::resized()
{
	auto ctl_rect = getLocalBounds();
	int ctl_win_width;
	int ctl_win_height;

	ctl_rect.reduce(BORDER_WIDTH, BORDER_WIDTH);
	ctl_rect.removeFromTop(4);

	ctl_win_height = ctl_rect.getHeight();
	ctl_win_width = ctl_rect.getWidth();

	record_file_name_label.setBounds(
			ctl_rect.removeFromTop(ctl_win_height / num_of_control_rows));
	auto record_labels_rect =
			ctl_rect.removeFromTop(ctl_win_height / num_of_control_rows);

	duration_title.setBounds(
			record_labels_rect.removeFromLeft(ctl_win_width / 3));
	duration_input.setBounds(
			record_labels_rect.removeFromLeft(((2 * ctl_win_width) / 3) / 3));
	duration_used.setBounds(record_labels_rect);

	auto record_buttons_rect =
				ctl_rect.removeFromTop(ctl_win_height / num_of_control_rows);
	startRecordButton.setBounds(
			record_buttons_rect.removeFromLeft(ctl_win_width / 3));
	stopRecordButton.setBounds(
			record_buttons_rect.removeFromLeft(ctl_win_width / 3));
	deleteRecordButton.setBounds(
			record_buttons_rect.removeFromLeft(ctl_win_width / 3));
}


void RecordGui::set_filename_str(const String &file_name_str)
{
	record_file_name_label.setText(file_name_str, juce::dontSendNotification);
}


void RecordGui::set_main_component(MainComponent *new_mainComponent)
{
	mainComponent = new_mainComponent;
}


void RecordGui::changeState(enum record_transport_state_e new_state)
{
	if (state != new_state)
	{
		state = new_state;

		switch (state)
		{
			case RECORD_STOPPED:
				stopRecordButton.setEnabled(false);
				startRecordButton.setEnabled(true);
				duration_input.setEnabled(true);
				break;

			case RECORD_RECORDING:
				stopRecordButton.setEnabled(true);
				startRecordButton.setEnabled(false);
				duration_input.setEnabled(false);
				break;
		}
	}
}


void RecordGui::update_rec_elapsed(int elapsed_duration_ms)
{
	double percent_used = (100 * elapsed_duration_ms) / rec_duration_ms;
	duration_used.setText(juce::String("recorded - ") +
		juce::String(percent_used, 2) + " %", juce::dontSendNotification);
}


#define MIN_DURATION  10
void RecordGui::startRecordButtonClicked()
{
	rec_duration_ms = duration_input.getText().getIntValue();
	if (rec_duration_ms < MIN_DURATION)
	{
		rec_duration_ms = MIN_DURATION;
	}
	duration_input.setText(String(rec_duration_ms), juce::dontSendNotification);
	mainComponent->change_recording_state(RECORD_STARTING, rec_duration_ms);
}

void RecordGui::stopRecordButtonClicked()
{
	mainComponent->change_recording_state(RECORD_STOPPING, 0);
}

void RecordGui::deleteRecordButtonClicked()
{
	//mainComponent->changeState(Record_STOPPING);
}
