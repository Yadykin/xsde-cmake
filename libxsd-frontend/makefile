# file      : makefile
# copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))build/bootstrap.make

default := $(out_base)/
test    := $(out_base)/.test
clean   := $(out_base)/.clean

$(default): $(out_base)/xsd-frontend/ $(out_base)/tests/
$(test): $(out_base)/tests/.test
$(clean): $(out_base)/xsd-frontend/.clean $(out_base)/tests/.clean

$(call import,$(src_base)/xsd-frontend/makefile)
$(call import,$(src_base)/tests/makefile)
