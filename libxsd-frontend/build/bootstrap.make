# file      : build/bootstrap.make
# copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

project_name := libxsd-frontend

# First try to include the bundled bootstrap.make if it exist. If that
# fails, let make search for the external bootstrap.make.
#
build := build-0.3

-include $(dir $(lastword $(MAKEFILE_LIST)))../../$(build)/bootstrap.make

ifeq ($(patsubst %build/bootstrap.make,,$(lastword $(MAKEFILE_LIST))),)
include $(build)/bootstrap.make
endif

# Configuration (optional; only used in the +dep packages).
#
$(call -include,$(dcf_root)/configuration-dynamic.make)

# Aliases
#
.PHONY: $(out_base)/       \
        $(out_base)/.test  \
        $(out_base)/.clean

ifdef %interactive%

.PHONY: test clean

test: $(out_base)/.test
clean: $(out_base)/.clean

ifneq ($(filter $(.DEFAULT_GOAL),test clean),)
.DEFAULT_GOAL :=
endif

endif

# Don't include dependency info for certain targets.
#
define include-dep
$(call -include,$1)
endef

ifneq ($(filter $(MAKECMDGOALS),clean disfigure),)
include-dep =
endif
