# https://www.kernel.org/doc/html/latest/kbuild/
# https://docs.kernel.org/bpf/btf.html

include globals.make

KBUILD_DIR ?= /lib/modules/$(shell uname -r)/build/

# For Kbuild files
export KBUILD_VERBOSE := 0

# M/KBUILD_EXTMOD (https://docs.kernel.org/kbuild/kbuild.html)
# Set the directory to look for the kernel source when building external modules.

# src (https://www.kernel.org/doc/html/latest/kbuild/makefiles.html)
# $(src) is the directory where the Makefile is located. Always use $(src) when
# referring to files located in the src tree.

.PHONY: all clean dmesg modinfo install uninstall

all:
	make -C $(KBUILD_DIR) M=$(TRDM_BUILD_DIR) src=$(TRDM_SOURCES_DIR) modules

clean:
	make -C $(KBUILD_DIR) M=$(TRDM_BUILD_DIR) src=$(TRDM_SOURCES_DIR) clean

dmesg:
	@dmesg --color=always | grep --color=no -Pzo '.+$(TRDM_MODULE).+(\n\s+.+)*' | tr '\0' '\n'

modinfo:
	modinfo $(TRDM_BUILD_DIR)/$(TRDM_MODULE).ko

install:
	insmod $(TRDM_BUILD_DIR)/$(TRDM_MODULE).ko

uninstall:
	rmmod $(TRDM_MODULE)
