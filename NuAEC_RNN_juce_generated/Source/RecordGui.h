#pragma once

#include <JuceHeader.h>

enum record_transport_state_e
{
	RECORD_STARTING,
	RECORD_RECORDING,
	RECORD_STOPPING,
	RECORD_STOPPED
};


class MainComponent;

class RecordGui : public GroupComponent
{
public:
	RecordGui();
	~RecordGui() override;

//	void paint (juce::Graphics& g) override;
	void resized() override;
	void set_filename_str(const juce::String &file_name_str);
	void set_main_component(MainComponent *new_mainComponent);
	void changeState(enum record_transport_state_e new_state);
	void update_rec_elapsed(int elapsed_duration_ms);
	int num_of_control_rows = 3;

private:
	juce::Label record_file_name_label;
	juce::Label duration_title;
	juce::Label duration_input;
	juce::Label duration_used;

	juce::TextButton startRecordButton;
	juce::TextButton stopRecordButton;
	juce::TextButton deleteRecordButton;
	enum record_transport_state_e state;
	MainComponent *mainComponent;
	int rec_duration_ms;

	void startRecordButtonClicked();
	void stopRecordButtonClicked();
	void deleteRecordButtonClicked();

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RecordGui)
};
