INCLUDE_THIS_COMPONENT := y


#DEFINES =

#CFLAGS =

#ASMFLAGS =


#INCLUDE_DIR +=$(APP_ROOT_DIR)/NuAEC_RNN_juce_generated/JuceLibraryCode


SRC += NuAEC_RNN_juce_generated/Source/Main.cpp
SRC += NuAEC_RNN_juce_generated/Source/MainComponent.cpp
SRC += NuAEC_RNN_juce_generated/Source/audio.cpp
SRC += NuAEC_RNN_juce_generated/Source/PlaybackGui.cpp

SRC += src/audio_topologies/NuAEC_rnn_chain.c
#SRC += AudioComponent.cpp
#SRC += Control_PC_App_Component.cpp
#SRC += cmd_play_file.c
#SRC += errors.c

SPEED_CRITICAL_FILES +=

VPATH = $(APP_ROOT_DIR)


include $(COMMON_CC)
