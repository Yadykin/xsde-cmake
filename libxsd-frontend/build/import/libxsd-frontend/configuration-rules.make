# file      : build/import/libxsd-frontend/configuration-rules.make
# copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

$(dcf_root)/import/libxsd-frontend/configuration-dynamic.make: | $(dcf_root)/import/libxsd-frontend/.
	$(call message,,$(scf_root)/import/libxsd-frontend/configure $@)

ifndef %foreign%

disfigure::
	$(call message,rm $(dcf_root)/import/libxsd-frontend/configuration-dynamic.make,\
rm -f $(dcf_root)/import/libxsd-frontend/configuration-dynamic.make)

endif
