INCLUDE_THIS_COMPONENT := y


#DEFINES =

#CFLAGS =

#ASMFLAGS =




SRC = app_dev.c
SRC += cmd_set_compressor.c
SRC += cmd_set_limiter.c
SRC += cmd_set_vb.c
SRC += cmd_set_vbf.c
SRC += cmd_set_vbi.c
SRC += cmd_set_3d.c
SRC += cmd_set_centeri.c
SRC += cmd_set_sidesi.c
SRC += cmd_ctl.c
SRC += cmd_set_eq_band.c
SRC += cmd_set_vol.c
SRC += cmd_set_i2s_loopback.c
SRC += cmd_set_cpu_stat_interval.c

SPEED_CRITICAL_FILES += app_dev.c

VPATH = src


include $(COMMON_CC)
