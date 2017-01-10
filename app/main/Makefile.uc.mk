INCLUDE_THIS_COMPONENT := y


#DEFINES =

#CFLAGS =

#ASMFLAGS =




SRC = Main.cpp
SRC += MainComponent.cpp
SRC += AudioComponent.cpp
SRC += Control_PC_App_Component.cpp
SRC += cmd_play_file.c
SRC += errors.c

SPEED_CRITICAL_FILES += Main.cpp MainComponent.cpp

VPATH = src


include $(COMMON_CC)
