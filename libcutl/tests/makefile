# file      : tests/makefile
# copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
# license   : MIT; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../build/bootstrap.make

tests := compiler container fs re shared-ptr

default   := $(out_base)/
test      := $(out_base)/.test
dist      := $(out_base)/.dist
clean     := $(out_base)/.clean

$(default): $(addprefix $(out_base)/,$(addsuffix /,$(tests)))
$(test): $(addprefix $(out_base)/,$(addsuffix /.test,$(tests)))

$(dist): export dirs :=
$(dist): #$(addprefix $(out_base)/,$(addsuffix /.dist,$(tests)))
	$(call meta-automake)

$(clean): $(addprefix $(out_base)/,$(addsuffix /.clean,$(tests)))

$(call include,$(bld_root)/dist.make)
$(call include,$(bld_root)/meta/automake.make)

$(foreach t,$(tests),$(call import,$(src_base)/$t/makefile))
