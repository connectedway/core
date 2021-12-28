/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_DOM_H__)
#define __OFC_DOM_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/persist.h"

/**
 * \defgroup Open Files XML Handling
 *
 * Open Files XML is an optional module used by the persist subsystem.  
 * Open Files provides a SAX and a DOM parser as well as a DOM printer.
 */

/**
 * \defgroup OFC_DOM DOM Parser
 * \ingroup persist
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
  OFC_CHAR *ns ;
  OFC_CHAR *nodeName ;
  OFC_CHAR *nodeValue ;
  DOM_NODE_TYPE nodeType ;
  struct dom_node *parentNode ;
  struct dom_node *firstChild ;
  struct dom_node *lastChild ;
  struct dom_node *previousSibling ;
  struct dom_node *nextSibling ;
  struct dom_node *attributes ;
  struct dom_node *ownerDocument ;
} OFC_DOMNode ;

typedef OFC_CHAR OFC_DOMString ;
typedef OFC_VOID OFC_DocumentType ;
typedef OFC_DOMNode *OFC_DOMNodelist ;
typedef OFC_DOMNode OFC_DOMDocument ;

#if defined(__cplusplus)
extern "C"
{
#endif
  OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_node(OFC_VOID);

  OFC_CORE_LIB OFC_DOMNode *
  ofc_dom_create_document (OFC_DOMString *namespaceURI,
                           OFC_DOMString *qualifiedName,
                           OFC_DocumentType *doctype) ;

  OFC_CORE_LIB OFC_DOMNode *
  ofc_dom_create_element (OFC_DOMNode *document,
                          const OFC_DOMString *name) ;

  OFC_CORE_LIB OFC_DOMNode *
  ofc_dom_create_cdata_section (OFC_DOMNode *document,
                                const OFC_DOMString *data) ;

  OFC_CORE_LIB OFC_DOMNode *
  ofc_dom_create_processing_instruction (OFC_DOMNode *document,
                                         const OFC_DOMString *target,
                                         const OFC_DOMString *data) ;

  OFC_CORE_LIB OFC_DOMString *
  ofc_dom_get_attribute (OFC_DOMNode *elem,
                         const OFC_DOMString *name) ;

  OFC_CORE_LIB OFC_VOID
  ofc_dom_set_attribute (OFC_DOMNode *elem,
                         const OFC_DOMString *attr,
                         const OFC_DOMString *value) ;

  OFC_CORE_LIB OFC_DOMNode *
  ofc_dom_append_child (OFC_DOMNode *document, OFC_DOMNode *child) ;

  OFC_CORE_LIB OFC_DOMNodelist *
  ofc_dom_get_elements_by_tag_name (OFC_DOMNode *node,
                                    const OFC_DOMString *name) ;

  OFC_CORE_LIB OFC_CHAR *
  ofc_dom_get_element_cdata (OFC_DOMNode *doc, OFC_CHAR *name) ;

  OFC_CORE_LIB OFC_CHAR *
  ofc_dom_get_element_cdata_unescape (OFC_DOMNode *doc, OFC_CHAR *name) ;

  OFC_CORE_LIB OFC_CHAR *
  ofc_dom_get_cdata (OFC_DOMNode *node) ;

  OFC_CORE_LIB OFC_ULONG
  ofc_dom_get_element_cdata_long (OFC_DOMNode *doc, OFC_CHAR *name) ;

  OFC_CORE_LIB OFC_DOMNode *
  ofc_dom_get_element (OFC_DOMNode *doc, const OFC_CHAR *name) ;

  OFC_CORE_LIB OFC_DOMNode *
  ofc_dom_create_element_cdata (OFC_DOMNode *doc, OFC_CHAR *name,
                                const OFC_CHAR *string) ;

  OFC_CORE_LIB OFC_VOID
  ofc_dom_destroy_node_list (OFC_DOMNodelist *nodelist) ;

  OFC_CORE_LIB OFC_VOID
  ofc_dom_destroy_document (OFC_DOMNode *node) ;

  OFC_CORE_LIB OFC_VOID
  ofc_dom_destroy_node (OFC_DOMNode *node) ;

  OFC_CORE_LIB OFC_SIZET
  ofc_dom_sprint_node (OFC_CHAR *buf, OFC_SIZET len,
                       OFC_DOMNode *node, OFC_INT level) ;

  OFC_CORE_LIB OFC_SIZET
  ofc_dom_sprint_document (OFC_CHAR *buf, OFC_SIZET len,
                           OFC_DOMNode *node) ;

  OFC_CORE_LIB OFC_DOMNode *
  ofc_dom_load_document (OFC_SIZET callback(OFC_VOID*, OFC_LPVOID,
                                            OFC_DWORD),
                         OFC_VOID *) ;
  OFC_CORE_LIB OFC_SIZET
  ofc_dom_unescape (OFC_CHAR* data) ;

#if defined(__cplusplus)
}
#endif

#endif
/** \} */
