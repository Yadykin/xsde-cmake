# file      : tests/schema/element-group/makefile
# copyright : Copyright (c) 2006-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../../../build/bootstrap.make

tests := 000 001 002

driver := $(out_root)/tests/dump/driver
test   := $(out_base)/.test
clean  := $(out_base)/.clean

# Convenience alias for default target.
#
$(out_base)/: $(driver)

# Test.
#
test_targets := $(addprefix $(out_base)/.test-,$(tests))

$(test): $(test_targets)
$(test_targets): driver := $(driver)

.PHONY: $(out_base)/.test-%
$(out_base)/.test-%: $(driver) $(src_base)/test-%.xsd $(src_base)/test-%.std
	$(call message,test $(out_base)/$*,$(driver) $(src_base)/test-$*.xsd | diff -u $(src_base)/test-$*.std -)

# Clean.
#
$(clean):

# Dependencies.
#
$(call import,$(src_root)/tests/dump/makefile)
