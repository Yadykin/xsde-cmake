# file      : xsde/makefile
# copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../build/bootstrap.make

cxx_tun := xsde.cxx

# C++.
#
cxx_tun += cxx/elements.cxx

# C++/Parser.
#
cxx_tun += cxx/parser/elements.cxx                        \
           cxx/parser/validator.cxx                       \
           cxx/parser/name-processor.cxx                  \
           cxx/parser/state-processor.cxx                 \
           cxx/parser/type-processor.cxx                  \
           cxx/parser/generator.cxx                       \
           cxx/parser/parser-forward.cxx                  \
           cxx/parser/parser-header.cxx                   \
           cxx/parser/parser-inline.cxx                   \
           cxx/parser/parser-source.cxx                   \
           cxx/parser/impl-header.cxx                     \
           cxx/parser/impl-source.cxx                     \
	   cxx/parser/driver-source.cxx                   \
           cxx/parser/element-validation-source.cxx       \
           cxx/parser/attribute-validation-source.cxx     \
           cxx/parser/characters-validation-source.cxx

# C++/Serializer.
#
cxx_tun += cxx/serializer/elements.cxx                    \
           cxx/serializer/validator.cxx                   \
           cxx/serializer/name-processor.cxx              \
           cxx/serializer/type-processor.cxx              \
           cxx/serializer/generator.cxx                   \
           cxx/serializer/serializer-forward.cxx          \
           cxx/serializer/serializer-header.cxx           \
           cxx/serializer/serializer-inline.cxx           \
           cxx/serializer/serializer-source.cxx           \
           cxx/serializer/impl-header.cxx                 \
           cxx/serializer/impl-source.cxx                 \
	   cxx/serializer/driver-source.cxx               \
           cxx/serializer/element-validation-source.cxx   \
           cxx/serializer/attribute-validation-source.cxx

# C++/Hybrid.
#
cxx_tun += cxx/hybrid/elements.cxx                        \
           cxx/hybrid/validator.cxx                       \
           cxx/hybrid/generator.cxx                       \
	   cxx/hybrid/default-value.cxx                   \
           cxx/hybrid/tree-size-processor.cxx             \
           cxx/hybrid/tree-name-processor.cxx             \
           cxx/hybrid/tree-forward.cxx                    \
           cxx/hybrid/tree-header.cxx                     \
           cxx/hybrid/tree-inline.cxx                     \
           cxx/hybrid/tree-source.cxx                     \
           cxx/hybrid/tree-type-map.cxx                   \
	   cxx/hybrid/insertion-header.cxx                \
	   cxx/hybrid/insertion-source.cxx                \
	   cxx/hybrid/extraction-header.cxx               \
	   cxx/hybrid/extraction-source.cxx               \
           cxx/hybrid/parser-name-processor.cxx           \
           cxx/hybrid/parser-header.cxx                   \
           cxx/hybrid/parser-source.cxx                   \
           cxx/hybrid/parser-aggregate-header.cxx         \
           cxx/hybrid/parser-aggregate-source.cxx         \
           cxx/hybrid/serializer-name-processor.cxx       \
           cxx/hybrid/serializer-header.cxx               \
           cxx/hybrid/serializer-source.cxx               \
           cxx/hybrid/serializer-aggregate-header.cxx     \
           cxx/hybrid/serializer-aggregate-source.cxx

# Type map.
#
cxx_tun += type-map/lexer.cxx  \
           type-map/parser.cxx

# Processing.
#
cxx_tun += processing/inheritance/processor.cxx

# Options file.
#
cli_tun := options.cli                \
           cxx/options.cli            \
           cxx/hybrid/options.cli     \
           cxx/parser/options.cli     \
           cxx/serializer/options.cli

#
#
cxx_obj  := $(addprefix $(out_base)/,$(cxx_tun:.cxx=.o) $(cli_tun:.cli=.o))
cxx_od   := $(cxx_obj:.o=.o.d)

xsde     := $(out_base)/xsde
dist     := $(out_base)/.dist
dist-win := $(out_base)/.dist-win
clean    := $(out_base)/.clean
install  := $(out_base)/.install

# Import.
#
$(call import,\
  $(scf_root)/import/cli/stub.make,\
  cli: cli,cli-rules: cli_rules)

$(call import,\
  $(scf_root)/import/libcutl/stub.make,\
  l: cutl.l,cpp-options: cutl.l.cpp-options)

$(call import,\
  $(scf_root)/import/libxsd-frontend/stub.make,\
  l: xsd_fe.l,cpp-options: xsd_fe.l.cpp-options)

# Build.
#
$(xsde): $(cxx_obj) $(xsd_fe.l) $(cutl.l)

$(cxx_obj) $(cxx_od): cpp_options := -I$(src_base)
$(cxx_obj) $(cxx_od):     \
  $(xsd_fe.l.cpp-options) \
  $(cutl.l.cpp-options)

genf := $(cli_tun:.cli=.hxx) $(cli_tun:.cli=.ixx) $(cli_tun:.cli=.cxx)
gen  := $(addprefix $(out_base)/,$(genf))

# Don't depend on installed executable since it may not exist.
#
ifneq ($(cli),cli)
$(gen): $(cli)
endif
$(gen): cli := $(cli)
# Watch out: if I add --generate-specifier, then I will need to fix
# code that relies on modifiers to also modify the "specified" flag.
#
$(gen): cli_options +=         \
-I $(src_base)                 \
--generate-modifier            \
--ostream-type ::std::wostream \
--exclude-base                 \
--suppress-undocumented        \
--generate-file-scanner        \
--include-with-brackets        \
--option-length 28             \
--cxx-prologue '\#include <options-parser.hxx>'

#--include-prefix xsde
#--guard-prefix XSDE

$(addprefix $(out_base)/cxx/options,.hxx .ixx .cxx): $(src_base)/options.cli
$(addprefix $(out_base)/cxx/options,.hxx .ixx .cxx): cli_options += \
--include-prefix cxx --guard-prefix CXX

$(addprefix $(out_base)/cxx/hybrid/options,.hxx .ixx .cxx): \
$(src_base)/options.cli $(src_base)/cxx/options.cli
$(addprefix $(out_base)/cxx/hybrid/options,.hxx .ixx .cxx): cli_options += \
--include-prefix cxx/hybrid --guard-prefix CXX_HYBRID

$(addprefix $(out_base)/cxx/parser/options,.hxx .ixx .cxx): \
$(src_base)/options.cli $(src_base)/cxx/options.cli
$(addprefix $(out_base)/cxx/parser/options,.hxx .ixx .cxx): cli_options += \
--include-prefix cxx/parser --guard-prefix CXX_PARSER

$(addprefix $(out_base)/cxx/serializer/options,.hxx .ixx .cxx): \
$(src_base)/options.cli $(src_base)/cxx/options.cli
$(addprefix $(out_base)/cxx/serializer/options,.hxx .ixx .cxx): cli_options += \
--include-prefix cxx/serializer --guard-prefix CXX_SERIALIZER

$(call include-dep,$(cxx_od),$(cxx_obj),$(gen))

# Alias for default target.
#
$(out_base)/: $(xsde)

# Dist.
#
$(dist): $(xsde)
	$(call install-exec,$<,$(dist_prefix)/bin/xsde)

$(dist-win): $(xsde)
	$(call install-exec,$<,$(dist_prefix)/bin/xsde.exe)

# Install.
#
$(install): $(xsde)
	$(call install-exec,$<,$(install_bin_dir)/xsde)

# Clean.
#
$(clean):                            \
  $(xsde).o.clean                    \
  $(addsuffix .cxx.clean,$(cxx_obj)) \
  $(addsuffix .cxx.clean,$(cxx_od))

ifneq ($(xsde_clean_gen),n)
$(clean): $(addprefix $(out_base)/,$(cli_tun:.cli=.cxx.cli.clean))
endif

# Generated .gitignore.
#
ifeq ($(out_base),$(src_base))
$(xsde): | $(out_base)/.gitignore

$(out_base)/.gitignore: files := xsde $(genf)
$(clean): $(out_base)/.gitignore.clean

$(call include,$(bld_root)/git/gitignore.make)
endif

# How to.
#
$(call include,$(cli_rules))
$(call include,$(bld_root)/cxx/o-e.make)
$(call include,$(bld_root)/cxx/cxx-o.make)
$(call include,$(bld_root)/cxx/cxx-d.make)
$(call include,$(bld_root)/install.make)
