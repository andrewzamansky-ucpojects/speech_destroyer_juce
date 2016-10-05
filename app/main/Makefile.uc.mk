INCLUDE_THIS_COMPONENT := y


#DEFINES =

#CFLAGS =

#ASMFLAGS =




SRC = Main.cpp
SRC += MainComponent.cpp


SPEED_CRITICAL_FILES += Main.cpp MainComponent.cpp

VPATH = src


include $(COMMON_CC)
