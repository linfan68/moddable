# ==================== START OF make.linemb.mk ====================
all: build

ifeq ("$(CPU)","arm64") # ARM64 toolchain
CC = /usr/bin/aarch64-linux-gnu-gcc
CXX = /usr/bin/aarch64-linux-gnu-g++
endif

ifeq ("$(CPU)","arm") # ARM toolchain
CC = /usr/bin/arm-linux-gnueabihf-gcc
CXX = /usr/bin/arm-linux-gnueabihf-g++
endif

ifeq ("$(CPU)","x86") # x86 toolchain
CC = gcc
CXX = g++
endif

ifndef CC # check if the toolchain is set
$(error CPU is not set. Please set CPU to arm, arm64 or x86)
endif

# -DINCLUDE_XSPLATFORM=1 \
# -DXSPLATFORM=\"linarm_xs.h\"

C_DEFINES = \
	-DINCLUDE_XSPLATFORM \
	-DXSPLATFORM=\"xsPlatform_linemb.h\" \
	-DXS_ARCHIVE=1 \
	-DmxRun=1 \
	-DmxNoFunctionLength=1 \
	-DmxNoFunctionName=1 \
	-DmxHostFunctionPrimitive=1 \
	-DmxFewGlobalsTable=1 \
	-DkCommodettoBitmapFormat=$(COMMODETTOBITMAPFORMAT) \
	-DkPocoRotation=$(POCOROTATION)

C_DEFINES += \
	-Wno-misleading-indentation \
	-Wno-implicit-fallthrough

C_FLAGS = -fPIC -c
LINK_LIBRARIES = -lpthread -lm -lc -ldl -latomic
LINK_OPTIONS = -fPIC


#
# Include the XS engine
#
XS_DIRECTORIES = \
	$(XS_DIR)/includes \
	$(XS_DIR)/sources \
	$(XS_DIR)/platforms/linemb \
	$(XS_DIR)/platforms/mc \
	$(XS_DIR)/platforms

XS_HEADERS = \
	$(XS_DIR)/platforms/mc/xsHosts.h \
	$(XS_DIR)/platforms/xsPlatform.h \
	$(XS_DIR)/includes/xs.h \
	$(XS_DIR)/includes/xsmc.h \
	$(XS_DIR)/sources/xsCommon.h \
	$(XS_DIR)/sources/xsAll.h \
	$(XS_DIR)/sources/xsScript.h

XS_OBJECTS = \
	$(LIB_DIR)/xsAll.c.o \
	$(LIB_DIR)/xsAPI.c.o \
	$(LIB_DIR)/xsArguments.c.o \
	$(LIB_DIR)/xsArray.c.o \
	$(LIB_DIR)/xsAtomics.c.o \
	$(LIB_DIR)/xsBigInt.c.o \
	$(LIB_DIR)/xsBoolean.c.o \
	$(LIB_DIR)/xsCode.c.o \
	$(LIB_DIR)/xsCommon.c.o \
	$(LIB_DIR)/xsDataView.c.o \
	$(LIB_DIR)/xsDate.c.o \
	$(LIB_DIR)/xsDebug.c.o \
	$(LIB_DIR)/xsError.c.o \
	$(LIB_DIR)/xsFunction.c.o \
	$(LIB_DIR)/xsGenerator.c.o \
	$(LIB_DIR)/xsGlobal.c.o \
	$(LIB_DIR)/xsJSON.c.o \
	$(LIB_DIR)/xsLexical.c.o \
	$(LIB_DIR)/xsMapSet.c.o \
	$(LIB_DIR)/xsMarshall.c.o \
	$(LIB_DIR)/xsMath.c.o \
	$(LIB_DIR)/xsMemory.c.o \
	$(LIB_DIR)/xsModule.c.o \
	$(LIB_DIR)/xsNumber.c.o \
	$(LIB_DIR)/xsObject.c.o \
	$(LIB_DIR)/xsPlatforms.c.o \
	$(LIB_DIR)/xsPromise.c.o \
	$(LIB_DIR)/xsProperty.c.o \
	$(LIB_DIR)/xsProxy.c.o \
	$(LIB_DIR)/xsRegExp.c.o \
	$(LIB_DIR)/xsRun.c.o \
	$(LIB_DIR)/xsScope.c.o \
	$(LIB_DIR)/xsScript.c.o \
	$(LIB_DIR)/xsSourceMap.c.o \
	$(LIB_DIR)/xsString.c.o \
	$(LIB_DIR)/xsSymbol.c.o \
	$(LIB_DIR)/xsSyntaxical.c.o \
	$(LIB_DIR)/xsTree.c.o \
	$(LIB_DIR)/xsType.c.o \
	$(LIB_DIR)/xsdtoa.c.o \
	$(LIB_DIR)/xsmc.c.o \
	$(LIB_DIR)/xsre.c.o \
	$(LIB_DIR)/xsHosts.c.o

MODULE_DIRS = \
	$(MODDABLE)/modules/base/timer\
	$(MODDABLE)/modules/base/instrumentation

C_INCLUDES += $(DIRECTORIES)
C_INCLUDES += $(foreach dir,$(XS_DIRECTORIES) $(TMP_DIR) $(MODULE_DIRS),-I$(dir))

# XS related targets
VPATH += $(XS_DIRECTORIES)

$(XS_OBJECTS) : $(XS_HEADERS)

$(LIB_DIR)/%.c.o: %.c
	@echo "# cc" $(<F)
	@$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) $< -o $@

$(TMP_DIR)/mc.xs.c.o: $(TMP_DIR)/mc.xs.c $(XS_HEADERS)
	@echo "# cc" $(<F)
	@$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) $< -o $@

$(TMP_DIR)/mc.xs.c: $(MODULES) $(MANIFEST)
	@echo "# xsl modules"
	@xsl -b $(MODULES_DIR) -o $(TMP_DIR) $(PRELOADS) $(STRIPS) $(CREATION) $(MODULES)

$(TMP_DIR)/mc.resources.o: $(TMP_DIR)/mc.resources.c
	@echo "# cc" $(<F)
	@$(CC) $< $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -c -o $@

$(TMP_DIR)/mc.resources.c: $(DATA) $(RESOURCES) $(MANIFEST) $(SDKCONFIG_H) 
	@echo "# mcrez resources"
	@mcrez $(DATA) $(RESOURCES) -o $(TMP_DIR) -r mc.resources.c

# The exectuable
$(TMP_DIR)/xs_main.o: $(BUILD_DIR)/devices/linemb/xsProj/main.c
	@echo "# cc" $(<F)
	@$(CC) $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -c $< -o $@

$(BIN_DIR)/$(NAME): ${XS_OBJECTS} $(TMP_DIR)/mc.xs.c.o $(OBJECTS) $(TMP_DIR)/xs_main.o $(TMP_DIR)/mc.resources.o
	@echo "# ld " $@
	$(CC) $(LINK_OPTIONS) $^ $(LINK_LIBRARIES) -static -o $@

build: $(PROJ_DIR) $(BIN_DIR)/$(NAME)

clean:
	echo "# Clean project"
	-rm -rf $(BIN_DIR) 2>/dev/null
	-rm -rf $(TMP_DIR) 2>/dev/null
	-rm -rf $(LIB_DIR) 2>/dev/null

# ==================== END OF make.linemb.mk ====================