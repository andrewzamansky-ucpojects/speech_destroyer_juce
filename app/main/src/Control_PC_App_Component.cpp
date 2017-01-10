
#include "AudioComponent.h"

extern "C" {

	void add_play_file_cmd();
	extern int run_command(const char *cmd, int flag);
	extern void delete_added_commands();

}

void AudioComponent::addControlCommands()
{
	add_play_file_cmd();
}

void AudioComponent::deleteControlCommands()
{
	delete_added_commands();
}

void AudioComponent::sendCommand(const char* cmd)
{
	run_command(cmd, 0);
}
