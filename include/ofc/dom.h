/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__BLUE_DOM_H__)
#define __BLUE_DOM_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/persist.h"

/**
 * \defgroup BlueConfig XML Handling
 *
 * Blue XML is an optional module used by the Blue Config subsystem.  
 * Blue XML provides a SAX and a DOM parser as well as a DOM printer.
 */

/**
 * \defgroup BlueDOM DOM Parser
 * \ingroup BlueConfig
 */

/** \{ */

typedef enum
  {
    ELEMENT_NODE = 1,
    ATTRIBUTE_NODE = 2,
    TEXT_NODE = 3,
    CDATA_SECTION_NODE = 4,
    ENTITY_REFERENCE_NODE = 5,
    ENTITY_NODE = 6,
    PROCESSING_INSTRUCTION_NODE = 7,
    COMMENT_NODE = 8,
    DOCUMENT_NODE = 9,
    DOCUMENT_TYPE_NODE = 10,
    DOCUMENT_FRAGMENT_NODE = 11,
    NOTATION_NODE = 12
  } DOM_NODE_TYPE ;

typedef struct dom_node
{
  BLUE_CHAR *ns ;
  BLUE_CHAR *nodeName ;
  BLUE_CHAR *nodeValue ;
  DOM_NODE_TYPE nodeType ;
  struct dom_node *parentNode ;
  struct dom_node *firstChild ;
  struct dom_node *lastChild ;
  struct dom_node *previousSibling ;
  struct dom_node *nextSibling ;
  struct dom_node *attributes ;
  struct dom_node *ownerDocument ;
} BLUE_DOMNode ;

typedef BLUE_CHAR BLUE_DOMString ;
typedef BLUE_VOID BLUE_DocumentType ;
typedef BLUE_DOMNode *BLUE_DOMNodelist ;
typedef BLUE_DOMNode BLUE_DOMDocument ;

#if defined(__cplusplus)
extern "C"
{
#endif
  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMcreateNode (BLUE_VOID) ;

  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMcreateDocument (BLUE_DOMString *namespaceURI, 
			 BLUE_DOMString *qualifiedName,
			 BLUE_DocumentType *doctype) ;

  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMcreateElement (BLUE_DOMNode *document, 
			const BLUE_DOMString *name) ;

  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMcreateCDATASection (BLUE_DOMNode *document, 
			     const BLUE_DOMString *data) ;

  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMcreateProcessingInstruction (BLUE_DOMNode *document, 
				      const BLUE_DOMString *target,
				      const BLUE_DOMString *data) ;

  BLUE_CORE_LIB BLUE_DOMString *
  BlueDOMgetAttribute (BLUE_DOMNode *elem, 
		       const BLUE_DOMString *name) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueDOMsetAttribute (BLUE_DOMNode *elem, 
		       const BLUE_DOMString *attr, 
		       const BLUE_DOMString *value) ;

  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMappendChild (BLUE_DOMNode *document, BLUE_DOMNode *child) ;

  BLUE_CORE_LIB BLUE_DOMNodelist *
  BlueDOMgetElementsByTagName (BLUE_DOMNode *node, 
			       const BLUE_DOMString *name) ;

  BLUE_CORE_LIB BLUE_CHAR *
  BlueDOMgetElementCDATA (BLUE_DOMNode *doc, BLUE_CHAR *name) ;

  BLUE_CORE_LIB BLUE_CHAR *
  BlueDOMgetElementCDATAUnescape (BLUE_DOMNode *doc, BLUE_CHAR *name) ;

  BLUE_CORE_LIB BLUE_CHAR *
  BlueDOMgetCDATA (BLUE_DOMNode *node) ;

  BLUE_CORE_LIB BLUE_ULONG 
  BlueDOMgetElementCDATAULong (BLUE_DOMNode *doc, BLUE_CHAR *name) ;

  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMgetElement (BLUE_DOMNode *doc, const BLUE_CHAR *name) ;

  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMcreateElementCDATA (BLUE_DOMNode *doc, BLUE_CHAR *name, 
			     const BLUE_CHAR *string) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueDOMdestroyNodelist (BLUE_DOMNodelist *nodelist) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueDOMdestroyDocument (BLUE_DOMNode *node) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueDOMdestroyNode (BLUE_DOMNode *node) ;

  BLUE_CORE_LIB BLUE_SIZET 
  BlueDOMsprintNode (BLUE_CHAR *buf, BLUE_SIZET len, 
		     BLUE_DOMNode *node, BLUE_INT level) ;

  BLUE_CORE_LIB BLUE_SIZET 
  BlueDOMsprintDocument (BLUE_CHAR *buf, BLUE_SIZET len, 
			 BLUE_DOMNode *node) ;

  BLUE_CORE_LIB BLUE_DOMNode *
  BlueDOMloadDocument (BLUE_SIZET callback(BLUE_VOID*, BLUE_LPVOID, 
					   BLUE_DWORD), 
		       BLUE_VOID *) ;
  BLUE_CORE_LIB BLUE_SIZET 
  BlueDOMUnescape (BLUE_CHAR* data) ;

#if defined(__cplusplus)
}
#endif

#endif
/** \} */
