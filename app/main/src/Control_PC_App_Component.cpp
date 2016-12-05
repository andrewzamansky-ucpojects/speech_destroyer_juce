
#include "AudioComponent.h"

extern "C" {

	void add_play_file_cmd();
	extern int run_command(const char *cmd, int flag);

}

void AudioComponent::addControlCommands()
{
	add_play_file_cmd();
}


void AudioComponent::sendCommand(const char* cmd)
{
	run_command(cmd, 0);
}
