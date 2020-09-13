char chain_inputs[] = "chain_inputs";
char pcm_splitter_audio[] = "pcm_splitter_audio";
char nuvoton_aec[] = "nuvoton_aec";
char rnn_right[] = "rnn_right";
char rnn_gain[] = "rnn_gain";
char passthrough_gain[] = "passthrough_gain";
char pcm_mixer[] = "pcm_mixer";
char chain_outputs[] = "chain_outputs";

static struct static_dsp_component_t chain[] = {
{ chain_inputs, CHAIN_INPUTS_DSPT ,
	SET_INPUTS(NO_INPUTS)},
{ nuvoton_aec, NUVOTON_AEC_DSPT ,
	SET_INPUTS( IN0(chain_inputs, 0), IN1(chain_inputs, 1),
				IN2(chain_inputs, 2), IN3(chain_inputs, 3)) },
{ rnn_right, RNN_DSPT ,
	SET_INPUTS( IN0(nuvoton_aec, 1) ) },
	{ rnn_gain, MULTIPLIER_1CH_DSPT ,
		SET_INPUTS( IN0(rnn_right, 0) ) },
{ passthrough_gain, MULTIPLIER_1CH_DSPT ,
	SET_INPUTS( IN0(chain_inputs, 0) ) },
{ chain_outputs, CHAIN_OUTPUTS_DSPT ,
	SET_INPUTS( IN0(passthrough_gain, 0), IN1(rnn_gain, 0) ) },
};
