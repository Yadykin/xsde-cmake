# file      : buildfile
# copyright : Copyright (c) 2009-2018 Code Synthesis Tools CC
# license   : MIT; see accompanying LICENSE file

./: {*/ -build/ -doc/ -m4/} doc{INSTALL LICENSE NEWS README} manifest

# Don't install tests or the INSTALL file.
#
tests/:          install = false
doc{INSTALL}@./: install = false
