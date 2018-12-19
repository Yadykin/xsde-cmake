cmake_minimum_required(VERSION 3.1)
project(xsde)

message("configuring libxsde...")

set(XSDE_LIBRARY_INSTALL_DIR CACHE PATH "Path where xsde library file should be installed to ")
set(XSDE_HEADERS_INSTALL_DIR CACHE PATH "Path where xsde library headers should be installed to ")

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(XSDE_ENCODING_UTF8 "" ON)
option(XSDE_STL "" ON)
option(XSDE_REGEXP "" OFF)
option(XSDE_EXCEPTIONS "" ON)
option(XSDE_PARSER_VALIDATION "" ON)
option(XSDE_SERIALIZER_VALIDATION "" ON)
option(XSDE_POLYMORPHIC "" ON)
option(XSDE_CUSTOM_ALLOCATOR "" OFF)
option(XSDE_DEFAULT_ALLOCATOR "" OFF) # default is relevant, if custom is set
option(XSDE_USE_CDR "" OFF)
option(XSDE_USE_XDR "" OFF)
option(XSDE_LONGLONG "" ON)

set(XSDE_ARCH_WIDTH 64)
set(XSDE_BYTEORDER 1234)
set(XSDE_REUSE_STYLE_MIXIN OFF)
set(XSDE_REUSE_STYLE_TIEIN ON)
set(XSDE_REUSE_STYLE_NONE OFF)

if (UNIX)
 set(XSDE_PLATFORM_POSIX ON)    
endif (UNIX)

if (XSDE_STL)
 set(XSDE_STL_ITERATOR ON)
 set(XSDE_IOSTREAM ON)
 set(XSDE_SNPRINTF ON)
else()
 set(XSDE_STL_ITERATOR OFF)
 set(XSDE_IOSTREAM OFF)
 set(XSDE_SNPRINTF OFF)
endif()

if (XSDE_ENCODING_UTF8)
 set(XSDE_ENCODING_ISO8859_1 OFF)
else()
 set(XSDE_ENCODING_ISO8859_1 ON)
endif()

if (XSDE_POLYMORPHIC)
 set(XSDE_PARSER_SMAP_BUCKETS 53)
 set(XSDE_PARSER_IMAP_BUCKETS 97)
 if (XSDE_PARSER_VALIDATION)
  set(XSDE_SERIALIZER_SMAP_BUCKETS 53)
 endif()
 set(XSDE_SERIALIZER_SMAP_BUCKET_BUCKETS 53)
 if (XSDE_SERIALIZER_VALIDATION)
  set(XSDE_SERIALIZER_IMAP_BUCKETS 97)
 endif()
endif()

configure_file(${PROJECT_SOURCE_DIR}/xsde/config.h.in ${CMAKE_CURRENT_BINARY_DIR}/xsde/config.h @ONLY)

if (XSDE_ENCODING_ISO8859_1)
 message("using iso8859-1 endcoding")
else()
 message("using UTF-8 endcoding")
endif()

if (XSDE_STL)
 message("using STL")
else()
 message("using STL")
endif()

if (XSDE_REGEXP)
 message("using regexp")
else()
 message("not using regexp")
endif()

if (XSDE_EXCEPTIONS)
 message("using exceptions")
else()
 message("not using exceptions")
endif()

if (XSDE_PARSER_VALIDATION)
 message("parser performs validation")
else()
 message("parser does not perform validation")
endif()

if (XSDE_SERIALIZER_VALIDATION)
 message("serializer performs validation")
else()
 message("serializer does not perform validation")
endif()

if (XSDE_POLYMORPHIC)
 message("Polymorphism aware")
else() 
 message("Polymorphism unaware")
endif()

if (XSDE_CUSTOM_ALLOCATOR)
 message("Using custom allocator")
 if (XSDE_DEFAULT_ALLOCATOR)
  message("with default allocator implementation")
 endif()
endif() 

if (XSDE_USE_CDR)
 message("using CDR")
else()
 message("not using CDR")
endif()

if (XSDE_USE_XDR)
 message("using XDR")
else()
 message("not using XDR")
endif()

if (XSDE_LONGLONG)
 message("supporting long long type")
else()
 message("not supporting long long type")
endif()


#set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/../../dist/lib)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -fopenmp")

link_directories(
    ${PROJECT_SOURCE_DIR}/../../dist/lib
  )

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${CMAKE_CURRENT_BINARY_DIR}
)
  
file(GLOB_RECURSE HEADERS
     RELATIVE ${PROJECT_SOURCE_DIR}
    "xsde/*.hxx"
    "xsde/*.ixx"
    "xsde/*.txx"
    "xsde/*.h")

##### BEGIN TESTFILES

set(SOURCES 
	xsde/c/expat/xmlparse.c 
	xsde/c/expat/xmlrole.c 
	xsde/c/expat/xmltok.c
 xsde/c/genx/genx.c 
 xsde/c/genx/char-props.c
)

#ifneq ($(xsde_parser_validation)$(xsde_serializer_validation),nn) # check twice if this Makefile abomination correctly transformed 
if (XSDE_PARSER_VALIDATION OR XSDE_SERIALIZER_VALIDATION) # to this (and below there is another instance)
 # if at least one of them is =y (is validating), then not equal and enter
 if (XSDE_REGEXP)
 set (SOURCES ${SOURCES}
  xsde/c/regexp/chvalid.c 
  xsde/c/regexp/xmlunicode.c 
  xsde/c/regexp/xmlregexp.c
 )
 endif()
endif()

if (XSDE_CUSTOM_ALLOCATOR)
 if (XSDE_DEFAULT_ALLOCATOR)
 set (SOURCES ${SOURCES}
  xsde/allocator.c
 )
 endif()
endif()

set (SOURCES ${SOURCES}
 xsde/cxx/string.cxx
 xsde/cxx/string-search.cxx
 xsde/cxx/ro-string.cxx
 xsde/cxx/stack.cxx
)

if (XSDE_CUSTOM_ALLOCATOR)
 if (XSDE_EXCEPTIONS)
 set (SOURCES ${SOURCES}
  xsde/cxx/allocator.cxx
 )
 endif()
endif()

if (XSDE_ENCODING_ISO8859_1)
set (SOURCES ${SOURCES}
 xsde/cxx/iso8859-1.cxx
)
endif()

if (NOT XSDE_STL)
set (SOURCES ${SOURCES}
 xsde/cxx/strdupx.cxx
)
endif()

if (NOT XSDE_EXCEPTIONS)
set (SOURCES ${SOURCES}
 xsde/cxx/sys-error.cxx
)
endif()

#ifneq ($(xsde_parser_validation)$(xsde_serializer_validation),nn)
if (XSDE_PARSER_VALIDATION OR XSDE_SERIALIZER_VALIDATION)
set (SOURCES ${SOURCES}
 xsde/cxx/schema-error.cxx
)
if (XSDE_REGEXP)
set (SOURCES ${SOURCES}
 xsde/cxx/schema-error.cxx
)
endif()

endif()

if (XSDE_POLYMORPHIC)
set (SOURCES ${SOURCES}
 xsde/cxx/hashmap.cxx
)
endif()

# Common data types.
#
set (SOURCES ${SOURCES}
 xsde/cxx/buffer.cxx 
 xsde/cxx/sequence-base.cxx
)

if (XSDE_STL)
set (SOURCES ${SOURCES}
 xsde/cxx/string-sequence-stl.cxx
)
else()
set (SOURCES ${SOURCES}
 xsde/cxx/qname.cxx 
 xsde/cxx/string-sequence.cxx
)
endif()


## XML
##
#ifneq ($(xsde_parser_validation)$(xsde_serializer_validation),nn)
if (XSDE_PARSER_VALIDATION OR XSDE_SERIALIZER_VALIDATION)
set (SOURCES ${SOURCES}
 xsde/cxx/xml/char-table.cxx 
 xsde/cxx/xml/ncname.cxx
)
endif()


## C++/Hybrid
##
set (SOURCES ${SOURCES}
 xsde/cxx/hybrid/any-type.cxx
 xsde/cxx/hybrid/any-type-pimpl.cxx
 xsde/cxx/hybrid/any-type-pskel.cxx
 xsde/cxx/hybrid/any-type-simpl.cxx
 xsde/cxx/hybrid/any-type-sskel.cxx
 xsde/cxx/hybrid/sequence.cxx
)

if (XSDE_POLYMORPHIC)
set (SOURCES ${SOURCES}
 xsde/cxx/hybrid/parser-map.cxx
 xsde/cxx/hybrid/serializer-map.cxx
)
endif()

if (XSDE_USE_CDR)
set (SOURCES ${SOURCES}
 xsde/cxx/hybrid/cdr/exceptions.cxx
 xsde/cxx/hybrid/cdr/istream.cxx
 xsde/cxx/hybrid/cdr/ostream.cxx
 xsde/cxx/hybrid/cdr/qname.cxx
 xsde/cxx/hybrid/cdr/date-time.cxx
 xsde/cxx/hybrid/cdr/string-sequence.cxx
)
endif()

if (XSDE_USE_XDR)
set (SOURCES ${SOURCES}
 xsde/cxx/hybrid/xdr/exceptions.cxx
 xsde/cxx/hybrid/xdr/istream.cxx
 xsde/cxx/hybrid/xdr/ostream.cxx
 xsde/cxx/hybrid/xdr/qname.cxx
 xsde/cxx/hybrid/xdr/date-time.cxx
 xsde/cxx/hybrid/xdr/string-sequence.cxx
)
endif()


## C++/Parser
##
set (SOURCES ${SOURCES}
 xsde/cxx/parser/elements.cxx
)

if (XSDE_POLYMORPHIC)
set (SOURCES ${SOURCES}
 xsde/cxx/parser/map.cxx 
 xsde/cxx/parser/substitution-map.cxx
)
endif()

if (XSDE_EXCEPTIONS)
set (SOURCES ${SOURCES}
 xsde/cxx/parser/exceptions.cxx
)
else()
set (SOURCES ${SOURCES}
 xsde/cxx/parser/error.cxx
)
endif()

set (SOURCES ${SOURCES}
 xsde/cxx/parser/expat/document.cxx 
 xsde/cxx/parser/expat/xml-error.cxx
)


# Parser implementations.
#
if (XSDE_PARSER_VALIDATION)
set (SOURCES ${SOURCES}
 xsde/cxx/parser/validating/parser.cxx
 xsde/cxx/parser/validating/time-zone.cxx
 xsde/cxx/parser/validating/string-common.cxx
 xsde/cxx/parser/validating/xml-schema-pskel.cxx
)
 if (XSDE_POLYMORPHIC)
 set (SOURCES ${SOURCES}
  xsde/cxx/parser/validating/inheritance-map.cxx
 )
 endif()

 set (SOURCES ${SOURCES}
  xsde/cxx/parser/validating/boolean.cxx
  xsde/cxx/parser/validating/number.cxx
  xsde/cxx/parser/validating/byte.cxx
  xsde/cxx/parser/validating/unsigned-byte.cxx
  xsde/cxx/parser/validating/short.cxx
  xsde/cxx/parser/validating/unsigned-short.cxx
  xsde/cxx/parser/validating/int.cxx
  xsde/cxx/parser/validating/unsigned-int.cxx
  xsde/cxx/parser/validating/integer.cxx
  xsde/cxx/parser/validating/negative-integer.cxx
  xsde/cxx/parser/validating/non-positive-integer.cxx
  xsde/cxx/parser/validating/positive-integer.cxx
  xsde/cxx/parser/validating/non-negative-integer.cxx
  xsde/cxx/parser/validating/float.cxx
  xsde/cxx/parser/validating/double.cxx
  xsde/cxx/parser/validating/decimal.cxx
 )

 if (XSDE_LONGLONG)
 set (SOURCES ${SOURCES}
  xsde/cxx/parser/validating/long-long.cxx
  xsde/cxx/parser/validating/unsigned-long-long.cxx
 )
 else()
 set (SOURCES ${SOURCES}
  xsde/cxx/parser/validating/long.cxx
  xsde/cxx/parser/validating/unsigned-long.cxx
 )
 endif()

 if (XSDE_STL)
 set (SOURCES ${SOURCES}
  xsde/cxx/parser/validating/any-simple-type-stl.cxx
  xsde/cxx/parser/validating/string-stl.cxx
  xsde/cxx/parser/validating/normalized-string-stl.cxx
  xsde/cxx/parser/validating/token-stl.cxx
  xsde/cxx/parser/validating/name-stl.cxx
  xsde/cxx/parser/validating/nmtoken-stl.cxx
  xsde/cxx/parser/validating/nmtokens-stl.cxx
  xsde/cxx/parser/validating/ncname-stl.cxx
  xsde/cxx/parser/validating/id-stl.cxx
  xsde/cxx/parser/validating/idref-stl.cxx
  xsde/cxx/parser/validating/idrefs-stl.cxx
  xsde/cxx/parser/validating/language-stl.cxx
  xsde/cxx/parser/validating/uri-stl.cxx
  xsde/cxx/parser/validating/qname-stl.cxx
 )

 else()

 set (SOURCES ${SOURCES}
  xsde/cxx/parser/validating/any-simple-type.cxx
  xsde/cxx/parser/validating/string.cxx
  xsde/cxx/parser/validating/normalized-string.cxx
  xsde/cxx/parser/validating/token.cxx
  xsde/cxx/parser/validating/name.cxx
  xsde/cxx/parser/validating/nmtoken.cxx
  xsde/cxx/parser/validating/nmtokens.cxx
  xsde/cxx/parser/validating/ncname.cxx
  xsde/cxx/parser/validating/id.cxx
  xsde/cxx/parser/validating/idref.cxx
  xsde/cxx/parser/validating/idrefs.cxx
  xsde/cxx/parser/validating/language.cxx
  xsde/cxx/parser/validating/uri.cxx
  xsde/cxx/parser/validating/qname.cxx
 )
 endif() # stl

 set (SOURCES ${SOURCES}
  xsde/cxx/parser/validating/base64-binary.cxx
  xsde/cxx/parser/validating/hex-binary.cxx
  xsde/cxx/parser/validating/gday.cxx
  xsde/cxx/parser/validating/gmonth.cxx
  xsde/cxx/parser/validating/gyear.cxx
  xsde/cxx/parser/validating/gmonth-day.cxx
  xsde/cxx/parser/validating/gyear-month.cxx
  xsde/cxx/parser/validating/date.cxx
  xsde/cxx/parser/validating/time.cxx
  xsde/cxx/parser/validating/date-time.cxx
  xsde/cxx/parser/validating/duration.cxx
 ) 

 else() # validation

 set (SOURCES ${SOURCES}
  xsde/cxx/parser/non-validating/parser.cxx
  xsde/cxx/parser/non-validating/time-zone.cxx
  xsde/cxx/parser/non-validating/string-common.cxx
  xsde/cxx/parser/non-validating/xml-schema-pskel.cxx
  xsde/cxx/parser/non-validating/boolean.cxx
  xsde/cxx/parser/non-validating/number.cxx
  xsde/cxx/parser/non-validating/byte.cxx
  xsde/cxx/parser/non-validating/unsigned-byte.cxx
  xsde/cxx/parser/non-validating/short.cxx
  xsde/cxx/parser/non-validating/unsigned-short.cxx
  xsde/cxx/parser/non-validating/int.cxx
  xsde/cxx/parser/non-validating/unsigned-int.cxx
  xsde/cxx/parser/non-validating/integer.cxx
  xsde/cxx/parser/non-validating/negative-integer.cxx
  xsde/cxx/parser/non-validating/non-positive-integer.cxx
  xsde/cxx/parser/non-validating/positive-integer.cxx
  xsde/cxx/parser/non-validating/non-negative-integer.cxx
  xsde/cxx/parser/non-validating/float.cxx
  xsde/cxx/parser/non-validating/double.cxx
  xsde/cxx/parser/non-validating/decimal.cxx
 )

 if (XSDE_LONGLONG)
 set (SOURCES ${SOURCES}
  xsde/cxx/parser/non-validating/long-long.cxx
  xsde/cxx/parser/non-validating/unsigned-long-long.cxx
 )
 else()
 set (SOURCES ${SOURCES}
  xsde/cxx/parser/non-validating/long.cxx
  xsde/cxx/parser/non-validating/unsigned-long.cxx
 )
 endif()

 if (XSDE_STL)

 set (SOURCES ${SOURCES}
  xsde/cxx/parser/non-validating/any-simple-type-stl.cxx
  xsde/cxx/parser/non-validating/string-stl.cxx
  xsde/cxx/parser/non-validating/normalized-string-stl.cxx
  xsde/cxx/parser/non-validating/token-stl.cxx
  xsde/cxx/parser/non-validating/name-stl.cxx
  xsde/cxx/parser/non-validating/nmtoken-stl.cxx
  xsde/cxx/parser/non-validating/nmtokens-stl.cxx
  xsde/cxx/parser/non-validating/ncname-stl.cxx
  xsde/cxx/parser/non-validating/id-stl.cxx
  xsde/cxx/parser/non-validating/idref-stl.cxx
  xsde/cxx/parser/non-validating/idrefs-stl.cxx
  xsde/cxx/parser/non-validating/language-stl.cxx
  xsde/cxx/parser/non-validating/uri-stl.cxx
  xsde/cxx/parser/non-validating/qname-stl.cxx
 )

 else()

 set (SOURCES ${SOURCES}
  xsde/cxx/parser/non-validating/any-simple-type.cxx
  xsde/cxx/parser/non-validating/string.cxx
  xsde/cxx/parser/non-validating/normalized-string.cxx
  xsde/cxx/parser/non-validating/token.cxx
  xsde/cxx/parser/non-validating/name.cxx
  xsde/cxx/parser/non-validating/nmtoken.cxx
  xsde/cxx/parser/non-validating/nmtokens.cxx
  xsde/cxx/parser/non-validating/ncname.cxx
  xsde/cxx/parser/non-validating/id.cxx
  xsde/cxx/parser/non-validating/idref.cxx
  xsde/cxx/parser/non-validating/idrefs.cxx
  xsde/cxx/parser/non-validating/language.cxx
  xsde/cxx/parser/non-validating/uri.cxx
  xsde/cxx/parser/non-validating/qname.cxx
 )

 endif() # stl

 set (SOURCES ${SOURCES}
  xsde/cxx/parser/non-validating/base64-binary.cxx
  xsde/cxx/parser/non-validating/hex-binary.cxx
  xsde/cxx/parser/non-validating/gday.cxx
  xsde/cxx/parser/non-validating/gmonth.cxx
  xsde/cxx/parser/non-validating/gyear.cxx
  xsde/cxx/parser/non-validating/gmonth-day.cxx
  xsde/cxx/parser/non-validating/gyear-month.cxx
  xsde/cxx/parser/non-validating/date.cxx
  xsde/cxx/parser/non-validating/time.cxx
  xsde/cxx/parser/non-validating/date-time.cxx
  xsde/cxx/parser/non-validating/duration.cxx
 )

endif() # validation

## C++/Serializer
##
set (SOURCES ${SOURCES}
 xsde/cxx/serializer/elements.cxx 
 xsde/cxx/serializer/context.cxx
)

if (XSDE_POLYMORPHIC)
set (SOURCES ${SOURCES}
 xsde/cxx/serializer/map.cxx 
 xsde/cxx/serializer/substitution-map.cxx
)
endif()

if (XSDE_EXCEPTIONS)
set (SOURCES ${SOURCES}
 xsde/cxx/serializer/exceptions.cxx
)
else()
set (SOURCES ${SOURCES}
 xsde/cxx/serializer/error.cxx
)
endif()

set (SOURCES ${SOURCES}
 xsde/cxx/serializer/genx/document.cxx 
 xsde/cxx/serializer/genx/xml-error.cxx
)

# Serializer implementations.
#
if (XSDE_SERIALIZER_VALIDATION)

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/validating/serializer.cxx
  xsde/cxx/serializer/validating/time-zone.cxx
  xsde/cxx/serializer/validating/string-common.cxx
  xsde/cxx/serializer/validating/xml-schema-sskel.cxx
 )

 if (XSDE_POLYMORPHIC)
 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/validating/inheritance-map.cxx
 )
 endif()

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/validating/boolean.cxx
  xsde/cxx/serializer/validating/byte.cxx
  xsde/cxx/serializer/validating/unsigned-byte.cxx
  xsde/cxx/serializer/validating/short.cxx
  xsde/cxx/serializer/validating/unsigned-short.cxx
  xsde/cxx/serializer/validating/int.cxx
  xsde/cxx/serializer/validating/unsigned-int.cxx
  xsde/cxx/serializer/validating/integer.cxx
  xsde/cxx/serializer/validating/negative-integer.cxx
  xsde/cxx/serializer/validating/non-positive-integer.cxx
  xsde/cxx/serializer/validating/positive-integer.cxx
  xsde/cxx/serializer/validating/non-negative-integer.cxx
  xsde/cxx/serializer/validating/float.cxx
  xsde/cxx/serializer/validating/double.cxx
  xsde/cxx/serializer/validating/decimal.cxx
 )

 if (XSDE_LONGLONG)
 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/validating/long-long.cxx
  xsde/cxx/serializer/validating/unsigned-long-long.cxx
 )
 else()
 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/validating/long.cxx
  xsde/cxx/serializer/validating/unsigned-long.cxx
 )
 endif()

 if (XSDE_STL)

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/validating/any-simple-type-stl.cxx
  xsde/cxx/serializer/validating/string-stl.cxx
  xsde/cxx/serializer/validating/normalized-string-stl.cxx
  xsde/cxx/serializer/validating/token-stl.cxx
  xsde/cxx/serializer/validating/name-stl.cxx
  xsde/cxx/serializer/validating/nmtoken-stl.cxx
  xsde/cxx/serializer/validating/ncname-stl.cxx
  xsde/cxx/serializer/validating/id-stl.cxx
  xsde/cxx/serializer/validating/idref-stl.cxx
  xsde/cxx/serializer/validating/language-stl.cxx
  xsde/cxx/serializer/validating/uri-stl.cxx
  xsde/cxx/serializer/validating/idrefs-stl.cxx
  xsde/cxx/serializer/validating/nmtokens-stl.cxx
  xsde/cxx/serializer/validating/qname-stl.cxx
 )

 else()

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/validating/any-simple-type.cxx
  xsde/cxx/serializer/validating/string.cxx
  xsde/cxx/serializer/validating/normalized-string.cxx
  xsde/cxx/serializer/validating/token.cxx
  xsde/cxx/serializer/validating/name.cxx
  xsde/cxx/serializer/validating/nmtoken.cxx
  xsde/cxx/serializer/validating/ncname.cxx
  xsde/cxx/serializer/validating/id.cxx
  xsde/cxx/serializer/validating/idref.cxx
  xsde/cxx/serializer/validating/language.cxx
  xsde/cxx/serializer/validating/uri.cxx
  xsde/cxx/serializer/validating/idrefs.cxx
  xsde/cxx/serializer/validating/nmtokens.cxx
  xsde/cxx/serializer/validating/qname.cxx
 )

 endif() # stl

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/validating/base64-binary.cxx
  xsde/cxx/serializer/validating/hex-binary.cxx
  xsde/cxx/serializer/validating/gday.cxx
  xsde/cxx/serializer/validating/gmonth.cxx
  xsde/cxx/serializer/validating/gyear.cxx
  xsde/cxx/serializer/validating/gmonth-day.cxx
  xsde/cxx/serializer/validating/gyear-month.cxx
  xsde/cxx/serializer/validating/date.cxx
  xsde/cxx/serializer/validating/time.cxx
  xsde/cxx/serializer/validating/date-time.cxx
  xsde/cxx/serializer/validating/duration.cxx
 )

 else() # validation

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/non-validating/serializer.cxx
  xsde/cxx/serializer/non-validating/time-zone.cxx
  xsde/cxx/serializer/non-validating/xml-schema-sskel.cxx
 )

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/non-validating/boolean.cxx
  xsde/cxx/serializer/non-validating/byte.cxx
  xsde/cxx/serializer/non-validating/unsigned-byte.cxx
  xsde/cxx/serializer/non-validating/short.cxx
  xsde/cxx/serializer/non-validating/unsigned-short.cxx
  xsde/cxx/serializer/non-validating/int.cxx
  xsde/cxx/serializer/non-validating/unsigned-int.cxx
  xsde/cxx/serializer/non-validating/integer.cxx
  xsde/cxx/serializer/non-validating/negative-integer.cxx
  xsde/cxx/serializer/non-validating/non-positive-integer.cxx
  xsde/cxx/serializer/non-validating/positive-integer.cxx
  xsde/cxx/serializer/non-validating/non-negative-integer.cxx
  xsde/cxx/serializer/non-validating/float.cxx
  xsde/cxx/serializer/non-validating/double.cxx
  xsde/cxx/serializer/non-validating/decimal.cxx
 )

 if (XSDE_LONGLONG)
 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/non-validating/long-long.cxx
  xsde/cxx/serializer/non-validating/unsigned-long-long.cxx
 )
 else()
 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/non-validating/long.cxx
  xsde/cxx/serializer/non-validating/unsigned-long.cxx
 )
 endif()

 if (XSDE_STL)

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/non-validating/any-simple-type-stl.cxx
  xsde/cxx/serializer/non-validating/string-stl.cxx
  xsde/cxx/serializer/non-validating/normalized-string-stl.cxx
  xsde/cxx/serializer/non-validating/token-stl.cxx
  xsde/cxx/serializer/non-validating/name-stl.cxx
  xsde/cxx/serializer/non-validating/nmtoken-stl.cxx
  xsde/cxx/serializer/non-validating/ncname-stl.cxx
  xsde/cxx/serializer/non-validating/id-stl.cxx
  xsde/cxx/serializer/non-validating/idref-stl.cxx
  xsde/cxx/serializer/non-validating/language-stl.cxx
  xsde/cxx/serializer/non-validating/uri-stl.cxx
  xsde/cxx/serializer/non-validating/idrefs-stl.cxx
  xsde/cxx/serializer/non-validating/nmtokens-stl.cxx
  xsde/cxx/serializer/non-validating/qname-stl.cxx
 )

 else()

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/non-validating/any-simple-type.cxx
  xsde/cxx/serializer/non-validating/string.cxx
  xsde/cxx/serializer/non-validating/normalized-string.cxx
  xsde/cxx/serializer/non-validating/token.cxx
  xsde/cxx/serializer/non-validating/name.cxx
  xsde/cxx/serializer/non-validating/nmtoken.cxx
  xsde/cxx/serializer/non-validating/ncname.cxx
  xsde/cxx/serializer/non-validating/id.cxx
  xsde/cxx/serializer/non-validating/idref.cxx
  xsde/cxx/serializer/non-validating/language.cxx
  xsde/cxx/serializer/non-validating/uri.cxx
  xsde/cxx/serializer/non-validating/idrefs.cxx
  xsde/cxx/serializer/non-validating/nmtokens.cxx
  xsde/cxx/serializer/non-validating/qname.cxx
 )

 endif() # stl

 set (SOURCES ${SOURCES}
  xsde/cxx/serializer/non-validating/base64-binary.cxx
  xsde/cxx/serializer/non-validating/hex-binary.cxx
  xsde/cxx/serializer/non-validating/gday.cxx
  xsde/cxx/serializer/non-validating/gmonth.cxx
  xsde/cxx/serializer/non-validating/gyear.cxx
  xsde/cxx/serializer/non-validating/gmonth-day.cxx
  xsde/cxx/serializer/non-validating/gyear-month.cxx
  xsde/cxx/serializer/non-validating/date.cxx
  xsde/cxx/serializer/non-validating/time.cxx
  xsde/cxx/serializer/non-validating/date-time.cxx
  xsde/cxx/serializer/non-validating/duration.cxx
 )

endif() # validation

##### END TESTFILES

add_library(xsde SHARED ${SOURCES} ${HEADERS})
  
set_target_properties(xsde PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/../../dist/lib)

if (XSDE_LIBRARY_INSTALL_DIR AND XSDE_HEADERS_INSTALL_DIR)
  install(TARGETS xsde 
           LIBRARY 
            DESTINATION ${XSDE_LIBRARY_INSTALL_DIR}
           )
  MACRO(INSTALL_HEADERS_WITH_DIRECTORY HEADER_LIST)
    FOREACH(HEADER ${${HEADER_LIST}})
        get_filename_component(DIRNAME ${HEADER} DIRECTORY)
        INSTALL(FILES ${HEADER} DESTINATION ${XSDE_HEADERS_INSTALL_DIR}/${DIRNAME}/${DIR})
    ENDFOREACH(HEADER)
  ENDMACRO(INSTALL_HEADERS_WITH_DIRECTORY)
  INSTALL_HEADERS_WITH_DIRECTORY(HEADERS) 
  
  INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/xsde/config.h 
           DESTINATION ${XSDE_HEADERS_INSTALL_DIR}/xsde)
  
else()
  message("Install target is not generated")
endif()
