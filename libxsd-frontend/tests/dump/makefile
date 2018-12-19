# file      : tests/dump/makefile
# copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../../build/bootstrap.make

cxx_tun := driver.cxx
cxx_obj := $(addprefix $(out_base)/,$(cxx_tun:.cxx=.o))
cxx_od  := $(cxx_obj:.o=.o.d)

xsd-fe.l             := $(out_root)/xsd-frontend/xsd-frontend.l
xsd-fe.l.cpp-options := $(out_root)/xsd-frontend/xsd-frontend.l.cpp-options

driver  := $(out_base)/driver
clean   := $(out_base)/.clean

# Build.
#
$(driver): $(cxx_obj) $(xsd-fe.l)

$(cxx_obj) $(cxx_od): $(xsd-fe.l.cpp-options)

$(call include-dep,$(cxx_od))

# Convenience alias for default target.
#
$(out_base)/: $(driver)

# Clean.
#
$(clean): $(driver).o.clean          \
  $(addsuffix .cxx.clean,$(cxx_obj)) \
  $(addsuffix .cxx.clean,$(cxx_od))

# Generated .gitignore.
#
ifeq ($(out_base),$(src_base))
$(driver): | $(out_base)/.gitignore

$(out_base)/.gitignore: files := driver
$(clean): $(out_base)/.gitignore.clean

$(call include,$(bld_root)/git/gitignore.make)
endif

# How to.
#
$(call include,$(bld_root)/cxx/o-e.make)
$(call include,$(bld_root)/cxx/cxx-o.make)
$(call include,$(bld_root)/cxx/cxx-d.make)


# Dependencies.
#
$(call import,$(src_root)/xsd-frontend/makefile)
