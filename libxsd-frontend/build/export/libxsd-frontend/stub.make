# file      : build/import/libxsd-frontend/stub.make
# copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

$(call include-once,$(src_root)/xsd-frontend/makefile,$(out_root))

$(call export,\
  l: $(out_root)/xsd-frontend/xsd-frontend.l,\
  cpp-options: $(out_root)/xsd-frontend/xsd-frontend.l.cpp-options)
