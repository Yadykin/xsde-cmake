# file      : makefile
# copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
# license   : MIT; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))build/bootstrap.make

dirs := cutl tests

default  := $(out_base)/
test     := $(out_base)/.test
dist     := $(out_base)/.dist
install  := $(out_base)/.install
clean    := $(out_base)/.clean

$(default): $(addprefix $(out_base)/,$(addsuffix /,$(dirs)))
$(test): $(addprefix $(out_base)/,$(addsuffix /.test,$(dirs)))

# No dist support for tests for now.
dist_dirs := $(filter-out tests,$(dirs))

$(dist): export dirs := $(dist_dirs)
$(dist): export docs := LICENSE NEWS README INSTALL version
$(dist): data_dist := libcutl-vc9.sln libcutl-vc10.sln
$(dist): exec_dist := bootstrap
$(dist): export extra_dist := $(data_dist) $(exec_dist)
$(dist): export version = $(shell cat $(src_root)/version)

$(dist): $(addprefix $(out_base)/,$(addsuffix /.dist,$(dist_dirs)))
	$(call dist-data,$(docs) $(data_dist) libcutl.pc.in)
	$(call dist-exec,$(exec_dist))
	$(call dist-dir,m4)
	$(call meta-automake)
	$(call meta-autoconf)

$(install): $(out_base)/cutl/.install
	$(call install-data,$(src_base)/LICENSE,$(install_doc_dir)/libcutl/LICENSE)
	$(call install-data,$(src_base)/NEWS,$(install_doc_dir)/libcutl/NEWS)
	$(call install-data,$(src_base)/README,$(install_doc_dir)/libcutl/README)

$(clean): $(addprefix $(out_base)/,$(addsuffix /.clean,$(dirs)))

$(call include,$(bld_root)/dist.make)
$(call include,$(bld_root)/install.make)
$(call include,$(bld_root)/meta/automake.make)
$(call include,$(bld_root)/meta/autoconf.make)

$(foreach d,$(dirs),$(call import,$(src_base)/$d/makefile))
