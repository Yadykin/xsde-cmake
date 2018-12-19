# file      : build/export/libcutl/stub.make
# copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
# license   : MIT; see accompanying LICENSE file

$(call include-once,$(src_root)/cutl/makefile,$(out_root))

$(call export,\
  l: $(out_root)/cutl/cutl.l,\
  cpp-options: $(out_root)/cutl/cutl.l.cpp-options)
