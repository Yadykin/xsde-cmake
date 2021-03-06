# file      : tests/cxx/serializer/generated-impl/makefile
# copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../../../../build/bootstrap.make

# Note that this test only checks compilation of the generated empty
# implementation and test driver. It may not work with some compilers
# (missing return statements).
#

xsd := test.xsd

obj := $(addprefix $(out_base)/,$(xsd:.xsd=-sskel.o) $(xsd:.xsd=-simpl.o) $(xsd:.xsd=-sdriver.o))
dep := $(obj:.o=.o.d)

xsde.l             := $(out_root)/libxsde/xsde/xsde.l
xsde.l.cpp-options := $(out_root)/libxsde/xsde/xsde.l.cpp-options

driver := $(out_base)/$(xsd:.xsd=-sdriver)
test   := $(out_base)/.test
clean  := $(out_base)/.clean


# Build.
#
$(driver): $(obj) $(xsde.l)

$(obj) $(dep): $(xsde.l.cpp-options)

genf := $(xsd:.xsd=-sskel.hxx) $(xsd:.xsd=-sskel.ixx) $(xsd:.xsd=-sskel.cxx) \
        $(xsd:.xsd=-simpl.hxx) $(xsd:.xsd=-simpl.cxx) $(xsd:.xsd=-sdriver.cxx)

gen := $(addprefix $(out_base)/,$(genf))

$(gen): xsde := $(out_root)/xsde/xsde
$(gen): xsde_options += --generate-empty-impl --generate-test-driver \
--force-overwrite
$(gen): $(out_root)/xsde/xsde

$(call include-dep,$(dep),$(obj),$(gen))

# Convenience alias for default target.
#
$(out_base)/: $(driver)


# Test.
#
$(test): driver := $(driver)
$(test): $(driver)
	$(call message,test $$1,:,$(driver))

# Clean.
#
$(clean): $(driver).o.clean                                  \
  $(addsuffix .cxx.clean,$(obj))                             \
  $(addsuffix .cxx.clean,$(dep))                             \
  $(addprefix $(out_base)/,$(xsd:.xsd=-sskel.cxx.xsd.clean)) \
  $(addprefix $(out_base)/,$(xsd:.xsd=-simpl.cxx.xsd.clean)) \
  $(addprefix $(out_base)/,$(xsd:.xsd=-sdriver.cxx.xsd.clean))


# Generated .gitignore.
#
ifeq ($(out_base),$(src_base))
$(gen): | $(out_base)/.gitignore
$(driver): | $(out_base)/.gitignore

$(out_base)/.gitignore: files := $(xsd:.xsd=-sdriver) $(genf)
$(clean): $(out_base)/.gitignore.clean

$(call include,$(bld_root)/git/gitignore.make)
endif


# How to.
#
$(call include,$(bld_root)/cxx/o-e.make)
$(call include,$(bld_root)/cxx/cxx-o.make)
$(call include,$(bld_root)/cxx/cxx-d.make)

xsd_simpl_suffix := -simpl
$(call include,$(scf_root)/xsde/serializer/xsd-cxx.make)


# Dependencies.
#
$(call import,$(src_root)/xsde/makefile)
$(call import,$(src_root)/libxsde/xsde/makefile)
