char chain_inputs[] = "chain_inputs";
char pcm_splitter_audio[] = "pcm_splitter_audio";
char nuvoton_aec[] = "nuvoton_aec";
char rnn_left[] = "rnn_left";
char rnn_gain[] = "rnn_gain";
char passthrough_gain[] = "passthrough_gain";
char pcm_mixer[] = "pcm_mixer";
char chain_outputs[] = "chain_outputs";
char downsample_filter_mic_left[] = "downsample_filter_mic_left";
char downsample_filter_mic_right[] = "downsample_filter_mic_right";
char downsample_filter_playback_left[] = "downsample_filter_playback_left";
char downsample_filter_playback_right[] = "downsample_filter_playback_right";

static struct static_dsp_component_t chain[] = {
{ chain_inputs, CHAIN_INPUTS_DSPT ,
	SET_INPUTS(NO_INPUTS)},
{ downsample_filter_mic_left, DOWNSAMPLING_BY_INT_DSPT,
		SET_INPUTS( IN0(chain_inputs, 0)) },
{ downsample_filter_mic_right, DOWNSAMPLING_BY_INT_DSPT,
		SET_INPUTS( IN0(chain_inputs, 1)) },
{ downsample_filter_playback_left, DOWNSAMPLING_BY_INT_DSPT,
		SET_INPUTS( IN0(chain_inputs, 2)) },
{ downsample_filter_playback_right, DOWNSAMPLING_BY_INT_DSPT,
		SET_INPUTS( IN0(chain_inputs, 3)) },
{ nuvoton_aec, NUVOTON_AEC_DSPT ,
	SET_INPUTS( IN0(downsample_filter_mic_left, 0),
				IN1(downsample_filter_mic_right, 0),
				IN2(downsample_filter_playback_left, 0),
				IN3(downsample_filter_playback_right, 0)) },
{ rnn_left, RNN_DSPT, SET_INPUTS( IN0(nuvoton_aec, 0) ) },
{ rnn_gain, MULTIPLIER_1CH_DSPT, SET_INPUTS( IN0(rnn_left, 0) ) },
{ passthrough_gain, MULTIPLIER_1CH_DSPT,
		SET_INPUTS( IN0(downsample_filter_mic_left, 0) ) },
{ chain_outputs, CHAIN_OUTPUTS_DSPT ,
	SET_INPUTS( IN0(passthrough_gain, 0), IN1(rnn_gain, 0) ) },
};
