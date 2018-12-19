# file      : xsd-frontend/makefile
# copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../build/bootstrap.make

cxx_tun := semantic-graph/annotation.cxx      \
           semantic-graph/any.cxx             \
           semantic-graph/any-attribute.cxx   \
           semantic-graph/attribute.cxx       \
           semantic-graph/attribute-group.cxx \
           semantic-graph/complex.cxx         \
           semantic-graph/compositors.cxx     \
           semantic-graph/element.cxx         \
           semantic-graph/element-group.cxx   \
           semantic-graph/elements.cxx        \
           semantic-graph/enumeration.cxx     \
           semantic-graph/fundamental.cxx     \
           semantic-graph/list.cxx            \
           semantic-graph/namespace.cxx       \
           semantic-graph/particle.cxx        \
           semantic-graph/schema.cxx          \
           semantic-graph/union.cxx

cxx_tun += traversal/attribute.cxx       \
           traversal/attribute-group.cxx \
           traversal/complex.cxx         \
           traversal/compositors.cxx     \
           traversal/element.cxx         \
           traversal/element-group.cxx   \
           traversal/elements.cxx        \
           traversal/enumeration.cxx     \
           traversal/fundamental.cxx     \
           traversal/list.cxx            \
           traversal/namespace.cxx       \
           traversal/particle.cxx        \
           traversal/schema.cxx          \
           traversal/union.cxx

cxx_tun += transformations/anonymous.cxx       \
           transformations/enum-synthesis.cxx  \
           transformations/restriction.cxx     \
           transformations/schema-per-type.cxx \
           transformations/simplifier.cxx

cxx_tun += generators/dependencies.cxx

cxx_tun += types.cxx parser.cxx schema-dom-parser.cxx


cxx_obj := $(addprefix $(out_base)/,$(cxx_tun:.cxx=.o))
cxx_od  := $(cxx_obj:.o=.o.d)
m4_cxx  := $(out_base)/semantic-graph/fundamental.hxx \
  $(out_base)/semantic-graph/fundamental.cxx

xsd_frontend.l             := $(out_base)/xsd-frontend.l
xsd_frontend.l.cpp-options := $(out_base)/xsd-frontend.l.cpp-options

clean   := $(out_base)/.clean

$(call import,\
  $(scf_root)/import/libxerces-c/stub.make,\
  l: xerces_c.l,cpp-options: xerces_c.l.cpp-options)

$(call import,\
  $(scf_root)/import/libcutl/stub.make,\
  l: cutl.l,cpp-options: cutl.l.cpp-options)

# What to build.
#
$(xsd_frontend.l): $(cxx_obj) $(cutl.l) $(xerces_c.l)

$(xsd_frontend.l.cpp-options): prefix := xsd-frontend/ $(out_root)/
$(xsd_frontend.l.cpp-options): value := -I$(src_root) -I$(out_root)
$(xsd_frontend.l.cpp-options): $(cutl.l.cpp-options)

#@@ This can be further optimized since only parser depends on xerces.
#
$(cxx_obj) $(cxx_od): $(xsd_frontend.l.cpp-options) $(xerces_c.l.cpp-options)

$(m4_cxx): $(src_base)/semantic-graph/fundamental.m4
$(m4_cxx): m4_options := -I $(src_base)/semantic-graph

$(call include-dep,$(cxx_od))

# Alias for default target.
#
$(out_base)/: $(xsd_frontend.l)

# Clean
#
$(clean): $(xsd_frontend.l).o.clean     \
  $(xsd_frontend.l.cpp-options).clean   \
  $(addsuffix .cxx.clean,$(cxx_obj))    \
  $(addsuffix .cxx.clean,$(cxx_od))

ifneq ($(libxsd_frontend_clean_gen),n)
$(clean): $(addsuffix .m4.clean,$(m4_cxx))
endif

# Generated .gitignore.
#
ifeq ($(out_base),$(src_base))
$(xsd_frontend.l): | $(out_base)/.gitignore

$(out_base)/.gitignore: files := semantic-graph/fundamental.hxx semantic-graph/fundamental.cxx
$(clean): $(out_base)/.gitignore.clean

$(call include,$(bld_root)/git/gitignore.make)
endif

# How to.
#
$(call include,$(bld_root)/cxx/o-l.make)
$(call include,$(bld_root)/cxx/cxx-o.make)
$(call include,$(bld_root)/cxx/cxx-d.make)
$(call include,$(bld_root)/m4/m4.make)
