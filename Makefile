all: dark flat

c_error     := $(shell tput sgr0; tput bold; tput setaf 1)
c_action    := $(shell tput sgr0; tput bold; tput setaf 12)
c_filename  := $(shell tput sgr0; tput setaf 5)
c_extension := $(shell tput sgr0; tput bold; tput setaf 2)
c_special   := $(shell tput sgr0; tput setaf 3)
c_default   := $(shell tput sgr0; tput setaf 7)

define pyesorex
	@echo -e '$(c_action)[pyesorex] Running recipe $(c_special)$1$(c_action) on $(c_filename)$(2)$(c_filename).sof$(c_default)'
	pyesorex --recipe-dir prototypes/recipes/ $(1) prototypes/sof/$(2).sof --log-level DEBUG
endef

dark:
	$(call pyesorex,metis_det_dark,masterdark)

flat:
	$(call pyesorex,metis_lm_img_flat,masterflat)

reduce:
	$(call pyesorex,metis_lm_basic_reduction,basicreduction)
