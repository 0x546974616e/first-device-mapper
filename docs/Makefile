.PHONY: all watch clean

F_INPUT = $(CURDIR)/main.typ
F_OUTPUT = $(CURDIR)/build/output.pdf
O_ROOT = --root $(CURDIR)
O_OPEN = --open
TYPST = typst

ifndef OPEN
OPEN = 0
endif

all:
	@mkdir -p $(dir $(F_OUTPUT))
	@$(TYPST) compile $(F_INPUT) $(F_OUTPUT) $(O_ROOT) \
		$(if $(filter 0,$(OPEN)),,$(O_OPEN))

watch:
	@mkdir -p $(dir $(F_OUTPUT))
	@$(TYPST) watch $(F_INPUT) $(F_OUTPUT) $(O_ROOT) $(O_OPEN)

clean:
	@test -f $(F_OUTPUT) && rm -v $(F_OUTPUT) || true
