# file      : build/import/libxsd-frontend/stub.make
# copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

$(call include-once,$(scf_root)/import/libxsd-frontend/configuration-rules.make,$(dcf_root))

libxsd_frontend_installed :=

$(call -include,$(dcf_root)/import/libxsd-frontend/configuration-dynamic.make)

ifdef libxsd_frontend_installed

ifeq ($(libxsd_frontend_installed),y)

$(call export,l: -lxsd-frontend -lcutl -lxerces-c,cpp_options: )

else

# Include export stub.
#
$(call include,$(scf_root)/export/libxsd-frontend/stub.make)

endif

else

.NOTPARALLEL:

endif
