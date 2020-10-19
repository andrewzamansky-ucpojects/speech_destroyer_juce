char chain_inputs[] = "chain_inputs";
char fir_filter[] = "fir_filter";
char chain_outputs[] = "chain_outputs";
char passthrough_gain[] = "passthrough_gain";

static struct static_dsp_component_t chain[] = {
{ chain_inputs, CHAIN_INPUTS_DSPT , SET_INPUTS(NO_INPUTS)},
{ fir_filter, FIR_FILTER_DSPT, SET_INPUTS( IN0(chain_inputs, 0)) },
{ passthrough_gain, MULTIPLIER_1CH_DSPT, SET_INPUTS( IN0(chain_inputs, 0) ) },
{ chain_outputs, CHAIN_OUTPUTS_DSPT,
	SET_INPUTS( IN0(passthrough_gain, 0), IN1(fir_filter, 0) ) },
};
