# file      : tests/cxx/parser/generated-impl/makefile
# copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../../../../build/bootstrap.make

cxx :=
xsd := test.xsd

obj := $(addprefix $(out_base)/,$(xsd:.xsd=-pskel.o) $(xsd:.xsd=-pimpl.o) $(xsd:.xsd=-pdriver.o))
dep := $(obj:.o=.o.d)

xsde.l             := $(out_root)/libxsde/xsde/xsde.l
xsde.l.cpp-options := $(out_root)/libxsde/xsde/xsde.l.cpp-options

driver   := $(out_base)/$(xsd:.xsd=-pdriver)
test     := $(out_base)/.test
dist     := $(out_base)/.dist
dist-win := $(out_base)/.dist-win
clean    := $(out_base)/.clean


# Build.
#
$(driver): $(obj) $(xsde.l)

$(obj) $(dep): $(xsde.l.cpp-options)

genf := $(xsd:.xsd=-pskel.hxx) $(xsd:.xsd=-pskel.ixx) $(xsd:.xsd=-pskel.cxx) \
        $(xsd:.xsd=-pimpl.hxx) $(xsd:.xsd=-pimpl.cxx) $(xsd:.xsd=-pdriver.cxx)

gen := $(addprefix $(out_base)/,$(genf))

$(gen): xsde := $(out_root)/xsde/xsde
$(gen) $(dist) $(dist-win): xsde_options += --generate-print-impl \
--generate-test-driver --force-overwrite
$(gen): $(out_root)/xsde/xsde

$(call include-dep,$(dep),$(obj),$(gen))

# Convenience alias for default target.
#
$(out_base)/: $(driver)


# Test.
#
$(test): driver := $(driver)
$(test): $(driver) $(src_base)/test-000.xml $(src_base)/test-000.std
	$(call message,test $$1,$$1 $(src_base)/test-000.xml | diff -u $(src_base)/test-000.std -,$(driver))


# Dist.
#
$(dist) $(dist-win): opt := -src $(src_base) -cmd cxx-parser -xsd "$(xsd)" \
-cxx "$(cxx)" -gen "$(genf)" -opt "$(xsde_options)" -out $(dist_prefix)

$(dist):
	$(call message,install $(src_base),$(scf_root)/dist $(opt))

$(dist-win):
	$(call message,install $(src_base),$(scf_root)/dist -win $(opt))


# Clean.
#
$(clean): $(driver).o.clean                                  \
  $(addsuffix .cxx.clean,$(obj))                             \
  $(addsuffix .cxx.clean,$(dep))                             \
  $(addprefix $(out_base)/,$(xsd:.xsd=-pskel.cxx.xsd.clean)) \
  $(addprefix $(out_base)/,$(xsd:.xsd=-pimpl.cxx.xsd.clean)) \
  $(addprefix $(out_base)/,$(xsd:.xsd=-pdriver.cxx.xsd.clean))


# Generated .gitignore.
#
ifeq ($(out_base),$(src_base))
$(gen): | $(out_base)/.gitignore
$(driver): | $(out_base)/.gitignore

$(out_base)/.gitignore: files := $(xsd:.xsd=-pdriver) $(genf)
$(clean): $(out_base)/.gitignore.clean

$(call include,$(bld_root)/git/gitignore.make)
endif


# How to.
#
$(call include,$(bld_root)/cxx/o-e.make)
$(call include,$(bld_root)/cxx/cxx-o.make)
$(call include,$(bld_root)/cxx/cxx-d.make)

xsd_pimpl_suffix := -pimpl
$(call include,$(scf_root)/xsde/parser/xsd-cxx.make)


# Dependencies.
#
$(call import,$(src_root)/xsde/makefile)
$(call import,$(src_root)/libxsde/xsde/makefile)
