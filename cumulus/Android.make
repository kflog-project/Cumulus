##################################################################
# Cumulus Android Makefile file for deploy preparation
#
# Copyright (c): 2012 by Axel Pauli
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

QT_LIB_DIR = $(NECESSITAS_ROOT)/Android/Qt/482/armeabi-v7a/lib

QT_JAR_DIR = $(NECESSITAS_ROOT)/Android/Qt/482/armeabi-v7a/jar

NDK_BIN_BIR = $(NECESSITAS_ROOT)/android-ndk/toolchains/arm-linux-androideabi-4.4.3/prebuilt/linux-x86/arm-linux-androideabi/bin

STRIP = $(NDK_BIN_BIR)/strip

QT_LIBS = libQtCore.so libQtGui.so libQtNetwork.so libQtXml.so

QT_JAR = QtIndustrius-14.jar

A_LIB_DIR = $(PROJECT_DIR)/android/libs/armeabi-v7a

ASSET_DIR = $(PROJECT_DIR)/android/assets

# Note that the Project directory must be passed to every make call!

usage:
	@echo "call: $(MAKE) -f Android.make PROJECT_DIR=<project-dir>"

copy:	check copy_qt copy_jar

copy_qt:	$(addprefix $(A_LIB_DIR)/, $(QT_LIBS))

$(addprefix $(A_LIB_DIR)/, $(QT_LIBS)):	$(addprefix $(QT_LIB_DIR)/, $(QT_LIBS))
	install --mode=755 $< $(A_LIB_DIR)
	$(STRIP) $@
	
copy_jar:	$(addprefix $(ASSET_DIR)/, $(QT_JAR))
	
$(addprefix $(ASSET_DIR)/, $(QT_JAR)):	$(QT_JAR_DIR)/$(QT_JAR)
	install --mode=644 $< $(ASSET_DIR)

check:
	@if [ -z "$(PROJECT_DIR)" ]; \
	then \
		echo "PROJECT_DIR is undefined"; \
		exit 1; \
	fi
	@test -d "$(A_LIB_DIR)" || mkdir -p $(A_LIB_DIR)

clean:
	rm -f $(addprefix $(A_LIB_DIR)/, $(QT_LIBS))
	rm -f $(addprefix $(ASSET_DIR)/, $(QT_JAR))
