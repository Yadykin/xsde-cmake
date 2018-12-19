// file      : xsd-frontend/parser.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_PARSER_HXX
#define XSD_FRONTEND_PARSER_HXX

#include <set>
#include <memory> // std::auto_ptr

#include <xsd-frontend/types.hxx>
#include <xsd-frontend/semantic-graph/schema.hxx>

namespace XSDFrontend
{
  struct InvalidSchema {};

  class LocationTranslator
  {
  public:
    virtual
    ~LocationTranslator ();

    virtual NarrowString
    translate (NarrowString const& location) = 0;
  };

  // Set of disabled warning IDs. Special ID "all" disables all
  // warnings.
  //
  typedef std::set<NarrowString> WarningSet;

  class Parser
  {
  public:
    ~Parser ();

    Parser (bool proper_restriction,
            bool multiple_imports,
            bool full_schema_check);

    Parser (bool proper_restriction,
            bool multiple_imports,
            bool full_schema_check,
            LocationTranslator&,
            const WarningSet& disabled);

  private:
    Parser (Parser const&);
    Parser& operator= (Parser const&);

  public:
    // Parse a schema file. Throws InvalidSchema in case of a failure.
    //
    std::auto_ptr<SemanticGraph::Schema>
    parse (SemanticGraph::Path const&);

    // Parse a number of schema files all into one semantic graph.
    // Each schema file is imported from an unnamed root translation
    // unit. Throws InvalidSchema in case of a failure.
    //
    std::auto_ptr<SemanticGraph::Schema>
    parse (SemanticGraph::Paths const&);

    // Returns a schema graph that corresponds to the XML Schema
    // namespace with built-in type definitions. The path is fake
    // and is only used as a lable.
    //
    std::auto_ptr<SemanticGraph::Schema>
    xml_schema (SemanticGraph::Path const&);

  private:
    class Impl;
    std::auto_ptr<Impl> impl_;
  };
}

#endif  // XSD_FRONTEND_PARSER_HXX
