
/*
 *  cmd_set_comressor.c
 */
#include "_project.h"


#include "u-boot/include/command.h"

#include "dsp_management_api.h"
#include "virtual_bass_api.h"


extern dsp_descriptor_t vb;

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
int do_set_vbi(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{

	set_vb_intensity_command(&vb,  argc,  argv );
	return 0;
}

U_BOOT_CMD(
	set_vbi,     255,	0,	do_set_vbi,
	"set_vbi  vol",
	"info   - \n"
);

int add_vbi_cmd()
{
	uboot_add_command(	&U_BOOT_CMD_VAR_NAME(set_vbi)	);
	return 0;
}
