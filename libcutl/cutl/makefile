# file      : cutl/makefile
# copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
# license   : MIT; see accompanying LICENSE file

include $(dir $(lastword $(MAKEFILE_LIST)))../build/bootstrap.make

sources   := exception.cxx shared-ptr/base.cxx

sources   += fs/exception.cxx fs/path.cxx fs/auto-remove.cxx

sources   += re/re.cxx

sources   +=         \
xml/qname.cxx        \
xml/parser.cxx       \
xml/serializer.cxx   \
xml/value-traits.cxx

sources   +=              \
compiler/context.cxx      \
compiler/type-info.cxx    \
compiler/cxx-indenter.cxx

boost_sources   :=                                \
details/boost/regex/src/regex.cxx                 \
details/boost/regex/src/usinstances.cxx           \
details/boost/regex/src/regex_raw_buffer.cxx      \
details/boost/regex/src/fileiter.cxx              \
details/boost/regex/src/cpp_regex_traits.cxx      \
details/boost/regex/src/instances.cxx             \
details/boost/regex/src/wide_posix_api.cxx        \
details/boost/regex/src/regex_debug.cxx           \
details/boost/regex/src/c_regex_traits.cxx        \
details/boost/regex/src/posix_api.cxx             \
details/boost/regex/src/wc_regex_traits.cxx       \
details/boost/regex/src/cregex.cxx                \
details/boost/regex/src/w32_regex_traits.cxx      \
details/boost/regex/src/regex_traits_defaults.cxx \
details/boost/regex/src/static_mutex.cxx          \
details/boost/regex/src/icu.cxx                   \
details/boost/regex/src/winstances.cxx

# Expat.
#
expat_sources := xmlparse.c xmlrole.c xmltok.c
expat_headers := expat.h expat_external.h 
expat_dist    := ascii.h asciitab.h config.h iasciitab.h internal.h \
latin1tab.h nametab.h utf8tab.h xmlrole.h xmltok.h xmltok_impl.c \
xmltok_impl.h xmltok_ns.c README LICENSE

expat_sources := $(addprefix details/expat/,$(expat_sources))
expat_headers := $(addprefix details/expat/,$(expat_headers))
expat_dist    := $(addprefix details/expat/,$(expat_dist))

# Genx.
#
genx_sources := details/genx/genx.c details/genx/char-props.c
genx_headers := details/genx/genx.h
genx_dist    := details/genx/README details/genx/LICENSE

c_tun     := $(expat_sources) $(genx_sources)
c_obj     := $(addprefix $(out_base)/,$(c_tun:.c=.o))
c_od      := $(c_obj:.o=.o.d)

cxx_tun   := $(sources) $(boost_sources)
cxx_obj   := $(addprefix $(out_base)/,$(cxx_tun:.cxx=.o))
cxx_od    := $(cxx_obj:.o=.o.d)

cutl.l             := $(out_base)/cutl.l
cutl.l.cpp-options := $(out_base)/cutl.l.cpp-options

default   := $(out_base)/
dist      := $(out_base)/.dist
install   := $(out_base)/.install
clean     := $(out_base)/.clean


# Build.
#
$(cutl.l): $(c_obj) $(cxx_obj)

$(c_obj) $(cxx_obj) $(c_od) $(cxx_od): $(cutl.l.cpp-options) $(out_base)/details/config.h
$(cutl.l.cpp-options): value := -I$(out_root) -I$(src_root)

$(call include,$(bld_root)/ld/configuration-lib.make) # ld_lib_type

$(out_base)/details/config.h: | $(out_base)/details/.
	@echo '/* file      : cutl/details/config.h'               >$@
	@echo ' * note      : automatically generated'            >>$@
	@echo ' */'                                               >>$@
	@echo '#ifndef CUTL_DETAILS_CONFIG_H'                     >>$@
	@echo '#define CUTL_DETAILS_CONFIG_H'                     >>$@
	@echo ''                                                  >>$@
	@echo '#define LIBCUTL_BYTEORDER 1234'                    >>$@
ifeq ($(ld_lib_type),archive)
	@echo '#define LIBCUTL_STATIC_LIB'                        >>$@
endif
	@echo ''                                                  >>$@
	@echo '#endif /* CUTL_DETAILS_CONFIG_H */'                >>$@

$(call include-dep,$(c_od) $(cxx_od),$(c_obj) $(cxx_obj),$(out_base)/details/config.h)

# Convenience alias for default target.
#
$(out_base)/: $(cutl.l)

# Dist.
#
$(dist): export sources := $(sources)
$(dist): export boost_sources := $(boost_sources)
$(dist): export expat_sources := $(expat_sources)
$(dist): export expat_headers := $(expat_headers)
$(dist): export genx_sources := $(genx_sources)
$(dist): export genx_headers := $(genx_headers)
$(dist): export headers := $(subst $(src_base)/,,$(shell find $(src_base) \
-name '*.hxx' -o -name '*.ixx' -o -name '*.txx'))
$(dist): data_dist := details/config-vc.h $(subst $(src_base)/,,$(shell find \
$(src_base)/details/boost -type f -name '*.hpp')) details/boost/LICENSE \
details/boost/README $(expat_dist) $(genx_dist)
$(dist): export extra_dist := $(data_dist) libcutl-vc9.vcproj \
libcutl-vc10.vcxproj libcutl-vc10.vcxproj.filters
$(dist): export interface_version = $(shell sed -e \
's/^\([0-9]*\.[0-9]*\).*/\1/' $(src_root)/version)

$(dist):
	$(call dist-data,$(sources) $(boost_sources) $(expat_sources) $(genx_sources))
	$(call dist-data,$(headers) $(expat_headers) $(genx_headers))
	$(call dist-data,$(data_dist) details/config.h.in)
	$(call meta-vc9proj,libcutl-vc9.vcproj)
	$(call meta-vc10proj,libcutl-vc10.vcxproj)
	$(call meta-automake)

# Install.
#
$(install): $(cutl.l)
	$(call install-lib,$<,$(install_lib_dir)/$(ld_lib_prefix)cutl$(ld_lib_suffix))
	$(call install-dir,$(src_base),$(install_inc_dir)/cutl,\
'(' -name '*.hxx' -o -name '*.ixx' -o -name '*.txx' ')')
	$(call install-data,$(out_base)/details/config.h,\
$(install_inc_dir)/cutl/details/config.h)

# Clean.
#
$(clean): $(cutl.l).o.clean          \
  $(cutl.l.cpp-options).clean        \
  $(addsuffix .cxx.clean,$(c_obj))   \
  $(addsuffix .cxx.clean,$(c_od))    \
  $(addsuffix .cxx.clean,$(cxx_obj)) \
  $(addsuffix .cxx.clean,$(cxx_od))
	$(call message,rm $$1,rm -f $$1,$(out_base)/details/config.h)

# Generated .gitignore.
#
ifeq ($(out_base),$(src_base))
$(cutl.l): | $(out_base)/.gitignore

$(out_base)/.gitignore: files := details/config.h
$(clean): $(out_base)/.gitignore.clean

$(call include,$(bld_root)/git/gitignore.make)
endif

# How to.
#
$(call include,$(bld_root)/dist.make)
$(call include,$(bld_root)/install.make)
$(call include,$(bld_root)/meta/vc9proj.make)
$(call include,$(bld_root)/meta/vc10proj.make)
$(call include,$(bld_root)/meta/automake.make)

$(call include,$(bld_root)/c/c-o.make)
$(call include,$(bld_root)/c/c-d.make)
$(call include,$(bld_root)/cxx/cxx-o.make)
$(call include,$(bld_root)/cxx/cxx-d.make)
$(call include,$(bld_root)/cxx/o-l.make)
