##################################################################
# Cumulus Android Makefile file for deploy preparation
#
# Copyright (c): 2012-2013 by Axel Pauli
#
# This file is distributed under the terms of the General Public
# License. See the file COPYING for more information.
#
# $Id$
#
# Note, that the SDK Necessitas is using this file for the
# deploy process.
#
##################################################################

NECESSITAS_ROOT = /opt/necessitas4A

QT_A_DIR = $(NECESSITAS_ROOT)/Android/Qt/482/armeabi-v7a

QT_LIB_DIR = $(QT_A_DIR)/lib

QT_PLUGIN_DIR = $(QT_A_DIR)/plugins/platforms/android

QT_JAR_DIR = $(QT_A_DIR)/jar

NDK_BIN_BIR = $(NECESSITAS_ROOT)/android-ndk/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/arm-linux-androideabi/bin

STRIP = $(NDK_BIN_BIR)/strip

QT_LIBS = libQtCore.so libQtGui.so libQtNetwork.so libQtXml.so

QT_PLUGIN = libandroid-9.so

QT_JAR = QtIndustrius-14.jar

CUMULUS = libCumulus.so

A_LIB_DIR = $(PROJECT_DIR)/android/libs/armeabi-v7a

ASSET_DIR = $(PROJECT_DIR)/android/assets

# Note that the Project directory must be passed to every make call!

usage:
	@echo "call: $(MAKE) -f Android.make PROJECT_DIR=<project-dir> BUILD_DIR=<build-dir>"

copy:	check copy_qt copy_plugin copy_jar copy_cumulus

copy_qt:	$(addprefix $(A_LIB_DIR)/, $(QT_LIBS))

$(addprefix $(A_LIB_DIR)/, $(QT_LIBS)):	$(addprefix $(QT_LIB_DIR)/, $(QT_LIBS))
	install --mode=755 $? $(A_LIB_DIR)
	$(STRIP) $(addprefix $(A_LIB_DIR)/, $(notdir $?))

copy_plugin:	$(addprefix $(A_LIB_DIR)/, $(QT_PLUGIN))

$(A_LIB_DIR)/$(QT_PLUGIN): $(QT_PLUGIN_DIR)/$(QT_PLUGIN)
	install --mode=755 $< $(A_LIB_DIR)
	$(STRIP) $(A_LIB_DIR)/$(QT_PLUGIN)

copy_jar:	$(ASSET_DIR)/$(QT_JAR)
	
$(ASSET_DIR)/$(QT_JAR):	$(QT_JAR_DIR)/$(QT_JAR)
	install --mode=644 $< $(ASSET_DIR)

copy_cumulus:
	$(STRIP) $(BUILD_DIR)/$(CUMULUS)
	install --mode=755 $(BUILD_DIR)/$(CUMULUS) $(A_LIB_DIR)
	
check:
	@if [ -z "$(PROJECT_DIR)" ]; \
	then \
		echo "PROJECT_DIR is undefined"; \
		exit 1; \
	fi
	@if [ -z "$(BUILD_DIR)" ]; \
	then \
		echo "BUILD_DIR is undefined"; \
		exit 1; \
	fi
	@test -d "$(A_LIB_DIR)" || mkdir -p $(A_LIB_DIR)

clean:
	rm -f $(addprefix $(A_LIB_DIR)/, $(QT_LIBS))
	rm -f $(A_LIB_DIR)/$(QT_PLUGIN)
	rm -f $(ASSET_DIR)/$(QT_JAR)
