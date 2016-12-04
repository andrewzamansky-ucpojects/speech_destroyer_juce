
/*
 *  cmd_set_comressor.c
 */
#include "_project.h"


#include "u-boot/include/command.h"
//#include "../JuceLibraryCode/JuceHeader.h"

//#include "AudioComponent.h"


//extern AudioComponent *AudioComponentObj;

void startToPlayFile(int argc, char * const argv[]);
void stopPlay();
void startPlay();

/*
 * Subroutine:  do_set_comressor
 *
 * Description:
 *
 * Inputs:
 *
 * Return:      None
 *
 */
int do_play_file(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	startToPlayFile(argc - 1, &argv[1]);
	return 0;
}

U_BOOT_CMD(
	play_file,     255,	0,	do_play_file,
	"play_file  file_name",
	"info   - \n"
);

int do_stop_play(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	stopPlay();
	return 0;
}

U_BOOT_CMD(
	stop_play, 255, 0, do_stop_play,
	"stop_play  ",
	"info   - \n"
	);

int do_start_play(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	startPlay();
	return 0;
}

U_BOOT_CMD(
	start_play, 255, 0, do_start_play,
	"start_play  ",
	"info   - \n"
	);


int add_play_file_cmd()
{
	uboot_add_command(&U_BOOT_CMD_VAR_NAME(play_file));
	uboot_add_command(&U_BOOT_CMD_VAR_NAME(stop_play));
	uboot_add_command(&U_BOOT_CMD_VAR_NAME(start_play));
	return 0;
}
