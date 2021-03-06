# file      : examples/cxx/hybrid/makefile
# copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../../../build/bootstrap.make

all_examples := \
allocator       \
binary          \
compositors     \
custom          \
evolution       \
hello           \
multiroot       \
polymorphism    \
polyroot        \
streaming       \
library         \
wildcard        \
filter          \
minimal

build_examples := binary compositors custom evolution

ifeq ($(xsde_iostream),y)
ifeq ($(xsde_exceptions),y)

build_examples += hello multiroot streaming

ifeq ($(xsde_stl),y)
build_examples += library wildcard filter
endif

ifeq ($(xsde_polymorphic),y)
build_examples += polymorphism polyroot
endif

endif
endif

ifeq ($(xsde_stl),n)
ifeq ($(xsde_exceptions),n)
build_examples += minimal

ifeq ($(xsde_custom_allocator),y)
build_examples += allocator
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
