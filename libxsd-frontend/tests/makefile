# file      : tests/makefile
# copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../build/bootstrap.make

default   := $(out_base)/
test      := $(out_base)/.test
clean     := $(out_base)/.clean

$(default): $(out_base)/dump/ $(out_base)/schema/
$(test): $(out_base)/schema/.test
$(clean): $(out_base)/dump/.clean $(out_base)/schema/.clean

$(call import,$(src_base)/dump/makefile)
$(call import,$(src_base)/schema/makefile)
