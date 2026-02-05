.PHONY: docs qemu

docs:
	@cd docs && $(MAKE)

# https://www.gnu.org/software/make/manual/make.html#Syntax-of-Functions
# https://blog.jgc.org/2007/06/escaping-comma-and-space-in-gnu-make.html

EMPTY :=
COMMA := ,
SPACE := $(EMPTY) $(EMPTY)
JOIN = $(subst $(SPACE),$2,$(strip $1))

VIRTFS += local
# VIRTFS += readonly=on
VIRTFS += path=$(CURDIR)/module
VIRTFS += security_model=mapped
VIRTFS += mount_tag=shared0
VIRTFS += id=shared0

qemu:
	/usr/bin/qemu-system-x86_64               \
		-hda $(CURDIR)/image/debian.qcow        \
		-virtfs $(call JOIN,$(VIRTFS),$(COMMA)) \
		-display default,show-cursor=on         \
		-m size=512M -enable-kvm
