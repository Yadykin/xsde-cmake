# file      : examples/cxx/parser/makefile
# copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../../../build/bootstrap.make

all_examples := generated hello library multiroot polymorphism polyroot \
mixed wildcard minimal

build_examples :=

ifneq ($(xsde_reuse_style),none)
build_examples += generated
endif

ifeq ($(xsde_stl),y)
ifeq ($(xsde_iostream),y)
ifeq ($(xsde_exceptions),y)

build_examples += hello

ifneq ($(xsde_reuse_style),none)
build_examples += library multiroot

ifeq ($(xsde_polymorphic),y)
build_examples += polymorphism polyroot
endif

endif

ifeq ($(xsde_reuse_style),tiein)
build_examples += mixed wildcard
endif

endif
endif
endif

ifeq ($(xsde_stl),n)
ifeq ($(xsde_exceptions),n)
ifneq ($(xsde_reuse_style),none)
build_examples += minimal
endif
endif
endif

default  := $(out_base)/
dist     := $(out_base)/.dist
dist-win := $(out_base)/.dist-win
clean    := $(out_base)/.clean

$(default): $(addprefix $(out_base)/,$(addsuffix /,$(build_examples)))
$(clean): $(addprefix $(out_base)/,$(addsuffix /.clean,$(build_examples)))

# Dist.
#
$(dist) $(dist-win): path := $(subst $(src_root)/,,$(src_base))

$(dist): $(addprefix $(out_base)/,$(addsuffix /.dist,$(all_examples)))
	$(call install-data,$(src_base)/README,$(dist_prefix)/$(path)/README)

$(dist-win): $(addprefix $(out_base)/,$(addsuffix /.dist-win,$(all_examples)))
	$(call install-data,$(src_base)/README,$(dist_prefix)/$(path)/README.txt)
	$(call message,,todos $(dist_prefix)/$(path)/README.txt)

ifneq ($(filter $(MAKECMDGOALS),dist dist-win),)
$(foreach e,$(all_examples),$(call import,$(src_base)/$e/makefile))
else
$(foreach e,$(build_examples),$(call import,$(src_base)/$e/makefile))
endif
