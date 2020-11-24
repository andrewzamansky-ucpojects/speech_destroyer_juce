char chain_inputs[] = "chain_inputs";
char downsample_filter[] = "downsample_filter";
char chain_outputs[] = "chain_outputs";

static struct static_dsp_component_t chain[] = {
{ chain_inputs, CHAIN_INPUTS_DSPT , SET_INPUTS(NO_INPUTS)},
{ downsample_filter, DOWNSAMPLING_BY_INT_DSPT,
		SET_INPUTS( IN0(chain_inputs, 0)) },
{ chain_outputs, CHAIN_OUTPUTS_DSPT,
		SET_INPUTS( IN0(downsample_filter, 0) ) },
};
