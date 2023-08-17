# If RACK_DIR is not defined when calling the Makefile, default to two directories above
RACK_DIR ?= ../..

# FLAGS will be passed to both the C and C++ compiler
FLAGS +=
CFLAGS +=
CXXFLAGS +=

# Careful about linking to shared libraries, since you can't assume much about the user's environment and library search path.
# Static libraries are fine, but they should be added to this plugin's build system.
LDFLAGS +=

# Add .cpp files to the build
SOURCES += $(wildcard src/*.cpp)

# Add files to the ZIP package when running `make dist`
# The compiled plugin and "plugin.json" are automatically added.
DISTRIBUTABLES += res
DISTRIBUTABLES += $(wildcard LICENSE*)

# Include the Rack plugin Makefile framework
include $(RACK_DIR)/plugin.mk

include test.mk

# Run Rack in development mode
# Mostly copied from https://github.com/dhemery/DHE-Modules/blob/main/Makefile

PLUGIN_ZIP_NAME := $(SLUG)-$(VERSION)-$(ARCH_NAME).vcvplugin
DIST_PLUGIN_ZIP := dist/$(PLUGIN_ZIP_NAME)
USER_PLUGIN_DIR := $(RACK_USER_DIR)/plugins
USER_PLUGIN_ZIP := $(USER_PLUGIN_DIR)/$(PLUGIN_ZIP_NAME)
USER_PLUGIN_MANIFEST =  $(USER_PLUGIN_DIR)/$(SLUG)/plugin.json
RACK_EXECUTABLE_PATH = $(RACK_APP)/Contents/MacOS/Rack

$(STAGING_DIR) $(USER_PLUGIN_DIR):
	mkdir -p $@

stage: dist $(USER_PLUGIN_DIR)
	cp $(DIST_PLUGIN_ZIP) $(USER_PLUGIN_DIR)

clean-stage:
	rm -rf $(USER_PLUGIN_DIR)/*

run: stage
	cd $(RACK_USER_DIR) && "$(RACK_EXECUTABLE_PATH)" --dev

run-unhidden: install-name-tool link-lib-rack stage-unhidden
	cd $(RACK_USER_DIR) && "$(RACK_EXECUTABLE_PATH)" --dev

stage-unhidden: install-name-tool link-lib-rack stage
	cd $(USER_PLUGIN_DIR) && tar xf $(PLUGIN_ZIP_NAME)
	rm $(USER_PLUGIN_ZIP)
	jq '.modules[].hidden=false' plugin.json > $(USER_PLUGIN_MANIFEST)

clean: clean-stage

# I don't know why I need this
install-name-tool:
	install_name_tool -change libRack.dylib "$(RACK_APP)/Contents/Resources/libRack.dylib" "$(RACK_USER_DIR)/plugins/$(SLUG)/plugin.dylib"

# I don't know why I need this
link-lib-rack:
	mkdir -p /tmp/Rack2/
	rm -f /tmp/Rack2/libRack.dylib
	ln -s "$(RACK_APP)/Contents/Resources/libRack.dylib" "/tmp/Rack2/libRack.dylib"
	
.PHONY: run clean-stage install-name-tool link-lib-rack
