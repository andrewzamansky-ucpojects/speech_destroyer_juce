#find current path :
APP_ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(firstword $(MAKEFILE_LIST)))))

#find build tools directory path :
ifneq ($(wildcard $(APP_ROOT_DIR)/../common/c/build_tools),) 
    BUILD_TOOLS_ROOT_DIR	:=	$(APP_ROOT_DIR)/../common/c/build_tools
else ifneq ($(wildcard $(APP_ROOT_DIR)/../../common/c/build_tools),) 
    BUILD_TOOLS_ROOT_DIR	:=	$(APP_ROOT_DIR)/../../common/c/build_tools
else ifneq ($(wildcard $(APP_ROOT_DIR)/../../../common/c/build_tools),) 
    BUILD_TOOLS_ROOT_DIR	:=	$(APP_ROOT_DIR)/../../../common/c/build_tools
else ifneq ($(wildcard $(APP_ROOT_DIR)/../../../../common/c/build_tools),) 
    BUILD_TOOLS_ROOT_DIR	:=	$(APP_ROOT_DIR)/../../../../common/c/build_tools
else
	$(error ---- BUILD TOOLS \(common/c/build_tools/build_tools\) DIR NOT FOUND ----)
endif

MAKEFILE_DEFS_ROOT_DIR := $(BUILD_TOOLS_ROOT_DIR)/makefiles

include $(MAKEFILE_DEFS_ROOT_DIR)/project_makefile.mk
