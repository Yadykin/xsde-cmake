// file      : xsd-frontend/schema-dom-parser.cxx
// copyright : Copyright (c) 2005-2017 Code Synthesis Tools CC
// license   : GNU GPL v2 + exceptions; see accompanying LICENSE file

#include <xsd-frontend/schema-dom-parser.hxx>

#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMAttr.hpp>
#include <xercesc/dom/DOMText.hpp>

#include <xercesc/framework/XMLValidityCodes.hpp>
#include <xercesc/validators/schema/SchemaSymbols.hpp>

#include <xercesc/util/XMLString.hpp>

#include <xercesc/internal/XMLScanner.hpp>
#include <xercesc/internal/ElemStack.hpp>

namespace XSDFrontend
{
  namespace XML
  {
    using namespace Xerces;

    const XMLCh line_key[2] = {chLatin_l, chNull};
    const XMLCh column_key[2] = {chLatin_c, chNull};

    SchemaDOMParser::
    SchemaDOMParser (MemoryManager* mgr)
        : XercesDOMParser(0, mgr, 0),
          depth_ (-1),
          ann_depth_ (-1),
          inner_ann_depth_ (-1)
    {
      error_reporter_.setErrorReporter (this);
      setValidationScheme (XercesDOMParser::Val_Never);
      setDoNamespaces (true);
    }

    void SchemaDOMParser::
    startElement (const XMLElementDecl& decl,
                  const unsigned int url_id,
                  const XMLCh* const prefix,
                  const RefVectorOf<XMLAttr>& attributes,
                  const XMLSize_t attr_count,
                  const bool empty,
                  const bool root)
    {
      depth_++;

      if (ann_depth_ == -1)
      {
        if (XMLString::equals(decl.getBaseName(),
                              SchemaSymbols::fgELT_ANNOTATION) &&
            XMLString::equals(getURIText(url_id),
                              SchemaSymbols::fgURI_SCHEMAFORSCHEMA))
        {
          ann_depth_ = depth_;
        }
      }
      else if (depth_ == ann_depth_ + 1)
      {
        inner_ann_depth_ = depth_;
      }

      XercesDOMParser::startElement (
        decl, url_id, prefix, attributes, attr_count, false, root);

      // Set the line/column info.
      //
      ReaderMgr::LastExtEntityInfo info;
      ((ReaderMgr*) fScanner->getLocator())->getLastExtEntityInfo(info);

      unsigned long l (static_cast<unsigned long> (info.lineNumber));
      unsigned long c (static_cast<unsigned long> (info.colNumber));

      fCurrentNode->setUserData (line_key, reinterpret_cast<void*> (l), 0);
      fCurrentNode->setUserData (column_key, reinterpret_cast<void*> (c), 0);

      // If an empty element, call the endElement() now.
      //
      if (empty)
        endElement (decl, url_id, root, prefix);
    }

    void SchemaDOMParser::
    endElement (const XMLElementDecl& decl,
                const unsigned int url_id,
                const bool root,
                const XMLCh* const prefix)
    {
      if(ann_depth_ > -1)
      {
        if (inner_ann_depth_ == depth_)
        {
          inner_ann_depth_ = -1;
        }
        else if (ann_depth_ == depth_)
        {
          ann_depth_ = -1;
        }
      }

      depth_--;

      XercesDOMParser::endElement (decl, url_id, root, prefix);
    }

    void SchemaDOMParser::
    docCharacters (const XMLCh* const s,
                   const XMLSize_t length,
                   const bool cdata)
    {
      // Ignore chars outside of content.
      //
      if (!fWithinElement)
        return;

      if (inner_ann_depth_ == -1)
      {
        if (!((ReaderMgr*) fScanner->getReaderMgr())->getCurrentReader()->
            isAllSpaces(s, length))
        {
          ReaderMgr::LastExtEntityInfo lastInfo;
          fScanner->getReaderMgr()->getLastExtEntityInfo(lastInfo);
          locator_.setValues(lastInfo.systemId, lastInfo.publicId,
                             lastInfo.lineNumber, lastInfo.colNumber);
          error_reporter_.emitError(XMLValid::NonWSContent,
                                    XMLUni::fgValidityDomain,
                                    &locator_);
        }
      }
      else
      {
        // When it's within either of the 2 annotation sub-elements,
        // characters are allowed and we need to store them.
        //
        XercesDOMParser::docCharacters (s, length, cdata);
      }
    }

    void SchemaDOMParser::
    docComment (const XMLCh* const)
    {
      // We don't want any comments.
    }

    void SchemaDOMParser::
    startEntityReference (const XMLEntityDecl&)
    {
    }

    void SchemaDOMParser::
    endEntityReference (const XMLEntityDecl&)
    {
    }

    void SchemaDOMParser::
    ignorableWhitespace (const XMLCh* const s,
                         const XMLSize_t length,
                         const bool cdata)
    {
      // Ignore chars before the root element.
      //
      if (!fWithinElement || !fIncludeIgnorableWhitespace)
        return;

      if (ann_depth_ > -1)
        XercesDOMParser::ignorableWhitespace (s, length, cdata);
    }
  }
}
