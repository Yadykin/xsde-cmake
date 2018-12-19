# file      : xsd-frontend/semantic-graph/fundamental.m4
# copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
# license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

define(`upcase', `translit(`$*', `a-z', `A-Z')')


define(`capitalize_word',
  `regexp(`$1', `^\(.\)\(.*\)', `upcase(`\1')`\2'')')


define(`capitalize',
  `patsubst(`$1', `\w+', `capitalize_word(`\&')')')

define(`make_class_name', `patsubst(capitalize(`$1'), ` ')')

define(`make_var_name', `patsubst(`$1', ` ', `_')')
