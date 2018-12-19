// file      : xsd-frontend/traversal/fundamental.hxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#ifndef XSD_FRONTEND_TRAVERSAL_FUNDAMENTAL_HXX
#define XSD_FRONTEND_TRAVERSAL_FUNDAMENTAL_HXX

#include <xsd-frontend/traversal/elements.hxx>
#include <xsd-frontend/semantic-graph/fundamental.hxx>

namespace XSDFrontend
{
  namespace Traversal
  {
    namespace Fundamental
    {
      typedef
      Node<SemanticGraph::Fundamental::Type>
      Type;

      // Integers.
      //
      typedef
      Node<SemanticGraph::Fundamental::Byte>
      Byte;

      typedef
      Node<SemanticGraph::Fundamental::UnsignedByte>
      UnsignedByte;

      typedef
      Node<SemanticGraph::Fundamental::Short>
      Short;

      typedef
      Node<SemanticGraph::Fundamental::UnsignedShort>
      UnsignedShort;

      typedef
      Node<SemanticGraph::Fundamental::Int>
      Int;

      typedef
      Node<SemanticGraph::Fundamental::UnsignedInt>
      UnsignedInt;

      typedef
      Node<SemanticGraph::Fundamental::Long>
      Long;

      typedef
      Node<SemanticGraph::Fundamental::UnsignedLong>
      UnsignedLong;

      typedef
      Node<SemanticGraph::Fundamental::Integer>
      Integer;

      typedef
      Node<SemanticGraph::Fundamental::NonPositiveInteger>
      NonPositiveInteger;

      typedef
      Node<SemanticGraph::Fundamental::NonNegativeInteger>
      NonNegativeInteger;

      typedef
      Node<SemanticGraph::Fundamental::PositiveInteger>
      PositiveInteger;

      typedef
      Node<SemanticGraph::Fundamental::NegativeInteger>
      NegativeInteger;


      // Boolean.
      //
      typedef
      Node<SemanticGraph::Fundamental::Boolean>
      Boolean;


      // Floats.
      //
      typedef
      Node<SemanticGraph::Fundamental::Float>
      Float;

      typedef
      Node<SemanticGraph::Fundamental::Double>
      Double;

      typedef
      Node<SemanticGraph::Fundamental::Decimal>
      Decimal;


      // Strings.
      //
      typedef
      Node<SemanticGraph::Fundamental::String>
      String;

      typedef
      Node<SemanticGraph::Fundamental::NormalizedString>
      NormalizedString;

      typedef
      Node<SemanticGraph::Fundamental::Token>
      Token;

      typedef
      Node<SemanticGraph::Fundamental::Name>
      Name;

      typedef
      Node<SemanticGraph::Fundamental::NameToken>
      NameToken;

      typedef
      Node<SemanticGraph::Fundamental::NameTokens>
      NameTokens;

      typedef
      Node<SemanticGraph::Fundamental::NCName>
      NCName;

      typedef
      Node<SemanticGraph::Fundamental::Language>
      Language;


      // Qualified name.
      //
      typedef
      Node<SemanticGraph::Fundamental::QName>
      QName;


      // ID/IDREF.
      //
      typedef
      Node<SemanticGraph::Fundamental::Id>
      Id;

      typedef
      Node<SemanticGraph::Fundamental::IdRef>
      IdRef;

      typedef
      Node<SemanticGraph::Fundamental::IdRefs>
      IdRefs;


      // URI.
      //
      typedef
      Node<SemanticGraph::Fundamental::AnyURI>
      AnyURI;


      // Binary.
      //
      typedef
      Node<SemanticGraph::Fundamental::Base64Binary>
      Base64Binary;

      typedef
      Node<SemanticGraph::Fundamental::HexBinary>
      HexBinary;


      // Date/time.
      //
      typedef
      Node<SemanticGraph::Fundamental::Date>
      Date;

      typedef
      Node<SemanticGraph::Fundamental::DateTime>
      DateTime;

      typedef
      Node<SemanticGraph::Fundamental::Duration>
      Duration;

      typedef
      Node<SemanticGraph::Fundamental::Day>
      Day;

      typedef
      Node<SemanticGraph::Fundamental::Month>
      Month;

      typedef
      Node<SemanticGraph::Fundamental::MonthDay>
      MonthDay;

      typedef
      Node<SemanticGraph::Fundamental::Year>
      Year;

      typedef
      Node<SemanticGraph::Fundamental::YearMonth>
      YearMonth;

      typedef
      Node<SemanticGraph::Fundamental::Time>
      Time;


      // Entity.
      //
      typedef
      Node<SemanticGraph::Fundamental::Entity>
      Entity;

      typedef
      Node<SemanticGraph::Fundamental::Entities>
      Entities;


      // Notation.
      //
      typedef
      Node<SemanticGraph::Fundamental::Notation>
      Notation;
    }
  }
}


#endif  // XSD_FRONTEND_TRAVERSAL_FUNDAMENTAL_HXX
