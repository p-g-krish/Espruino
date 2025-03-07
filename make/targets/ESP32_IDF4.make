ESP_ZIP     = $(PROJ_NAME).tgz

CMAKEFILE = $(BINDIR)/main/CMakeLists.txt
# 'gen' has a relative path - get rid of it and add it manually
INCLUDE_WITHOUT_GEN = $(subst -Igen,,$(INCLUDE)) -I$(ROOT)/gen

ifeq ($(CHIP),ESP32C3)
	SDKCONFIG = sdkconfig_c3
	FMW_BIN_NAME = espruino-esp32c3
	PORT ?= /dev/ttyACM0
else 
	ifeq ($(CHIP),ESP32)
		SDKCONFIG = sdkconfig
		FMW_BIN_NAME = espruino-esp32
		PORT ?= /dev/ttyUSB0
	else
		ifeq ($(CHIP),ESP32S3)
			SDKCONFIG = sdkconfig_s3
			FMW_BIN_NAME = espruino-esp32s3
			PORT ?= /dev/ttyUSB0
		else
			$(error Unknown ESP32 chip)
		endif
	endif
endif

$(CMAKEFILE):
	@mkdir -p $(BINDIR)/main # create directory if it doesn't exist
	@echo "MAKE CMAKEFILE"
	@echo "$(INCLUDE_WITHOUT_GEN)"
	@echo "idf_component_register(" > $(CMAKEFILE)
	@echo "						 SRCS" >> $(CMAKEFILE)
	@echo "						$(patsubst %,\"$(ROOT)/%\"\n						,$(SOURCES))" >> $(CMAKEFILE)
	@echo "						 INCLUDE_DIRS" >> $(CMAKEFILE)
	@echo "						$(patsubst -I%,\"%/\"\n						,$(INCLUDE_WITHOUT_GEN))" >> $(CMAKEFILE)
	@echo "						 )" >> $(CMAKEFILE)
	@echo "" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_TARGET} PUBLIC -DESP_IDF_VERSION_MAJOR=4)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_TARGET} PUBLIC $(DEFINES))" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_TARGET} PUBLIC -Og -fno-strict-aliasing -ffunction-sections -fdata-sections -fstrict-volatile-bitfields -fgnu89-inline  -nostdlib -MMD -MP -Wno-enum-compare)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-pointer-sign)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-implicit-int)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-maybe-uninitialized)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-return-type)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-switch)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-unused-variable)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-unused-function)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-unused-but-set-variable)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-cast-function-type)" >> $(CMAKEFILE)
	@echo "target_compile_options($$""{COMPONENT_LIB} PRIVATE -Wno-format)" >> $(CMAKEFILE)


$(PROJ_NAME).bin: $(CMAKEFILE) $(PLATFORM_CONFIG_FILE) $(PININFOFILE).h $(PININFOFILE).c $(WRAPPERFILE)
	$(Q)cp ${ROOT}/targets/esp32/IDF4/${SDKCONFIG} $(BINDIR)/sdkconfig
	$(Q)cp ${ROOT}/targets/esp32/IDF4/CMakeLists.txt $(BINDIR)
	$(Q)cp ${ROOT}/targets/esp32/IDF4/partitions.csv $(BINDIR)
	cd $(BINDIR) && idf.py build
	$(Q)cp $(BINDIR)/build/espruino.bin $(PROJ_NAME).bin

$(ESP_ZIP): $(PROJ_NAME).bin
	$(Q)rm -rf $(PROJ_NAME)
	$(Q)mkdir -p $(PROJ_NAME)
	$(Q)cp $(PROJ_NAME).bin $(PROJ_NAME)/$(FMW_BIN_NAME).bin
	$(Q)cp $(BINDIR)/build/partition_table/partition-table.bin $(PROJ_NAME)/partition-table.bin
	$(Q)cp $(BINDIR)/build/bootloader/bootloader.bin $(PROJ_NAME)/bootloader.bin
	$(Q)cp targets/esp32/README_flash.txt $(PROJ_NAME)
	$(Q)cp targets/esp32/README_flash_C3.txt $(PROJ_NAME)
	$(Q)cp targets/esp32/README_flash_S3.txt $(PROJ_NAME)
	$(Q)$(TAR) -zcf $(ESP_ZIP) $(PROJ_NAME) --transform='s/$(BINDIR)\///g'
	@echo "Created $(ESP_ZIP)"

proj: $(ESP_ZIP)
#depend on $(PROJ_NAME).bin

flash: $(PROJ_NAME).bin
	cd $(BINDIR) && idf.py flash -p $(PORT)

# flashes but also automatically runs a terminal app right after
# Use Ctrl-] to exit
flashmonitor: $(PROJ_NAME).bin
	cd $(BINDIR) && idf.py flash -p $(PORT)
	cd $(BINDIR) && idf.py monitor -p $(PORT)
