#find current path :
APP_ROOT_DIR := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))

#recursively(upward) find directory that contains $(SEARCH_DIR)
SEARCH_DIR :=common_public/build_tools/makefiles/c
parent =$(filter-out $(1),$(abspath $1/..))#stop in root directory
r =$(wildcard $1/$(SEARCH_DIR)) $(foreach d, $(call parent,$1), $(call r,$d))
COMMON_PROJECT_MAKEFILE_DIR :=$(firstword  $(call r,$(APP_ROOT_DIR)) )

ifeq ($(COMMON_PROJECT_MAKEFILE_DIR),)
    $(info !--- $(SEARCH_DIR) DIR NOT FOUND ----)
    $(error )
endif

include $(COMMON_PROJECT_MAKEFILE_DIR)/project_makefile.mk
