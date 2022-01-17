/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_DOM_H__)
#define __OFC_DOM_H__

#include "ofc/core.h"
#include "ofc/types.h"

/**
 * \defgroup dom DOM Parser
 *
 * Open Files uses an xml syntax for its persistent configuration.  These
 * routines are used to create, parse, and print XML documents.  The
 * memory representation of the XML files use the Document Object Model
 * (DOM).
 *
 * Although used for persistent configuration, applications can manage
 * their own dom structures and XML documents for either additional 
 * configuration files, or for network messages.
 *
 * These are minimally documented since, for the most part, these routines
 * are internal to the persistance handling.  These will be documented
 * further upon request.
 */

/** \{ */

/**
 * Internal DOM Node Types
 */
typedef enum {
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
} DOM_NODE_TYPE;

/**
 * DOM Node Structure
 *
 * A DOM is a network of DOM Nodes linked together.  As documents are parsed
 * the network is created and as documents are printed, the network is 
 * walked.
 */
typedef struct dom_node {
    OFC_CHAR *ns;
    OFC_CHAR *nodeName;
    OFC_CHAR *nodeValue;
    DOM_NODE_TYPE nodeType;
    struct dom_node *parentNode;
    struct dom_node *firstChild;
    struct dom_node *lastChild;
    struct dom_node *previousSibling;
    struct dom_node *nextSibling;
    struct dom_node *attributes;
    struct dom_node *ownerDocument;
} OFC_DOMNode;

/**
 * A DOMString is equivalent to a character
 */
typedef OFC_CHAR OFC_DOMString;
/**
 * Document Type is opaque
 */
typedef OFC_VOID OFC_DocumentType;
/**
 * A list of document nodes
 */
typedef OFC_DOMNode *OFC_DOMNodelist;
/**
 * A DOM Document is represented as a single root node
 */
typedef OFC_DOMNode OFC_DOMDocument;

#if defined(__cplusplus)
extern "C"
{
#endif
/**
 * Create an empty DOM node
 *
 * \returns
 * Pointer to a freshly allocated node
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_node(OFC_VOID);
/**
 * Create a DOM Document
 * 
 * This will consist of a single DOM Node of type DOCUMENT_TYPE and 
 * name ROOT.
 *
 * \param namespaceURI
 * Unused and can be NULL
 *
 * \param qualifiedName
 * Unused and can be NULL
 *
 * \param doctype
 * Unused and can be NULL
 *
 * \returns
 * Pointer to DOM Document
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_document(OFC_DOMString *namespaceURI,
                        OFC_DOMString *qualifiedName,
                        OFC_DocumentType *doctype);

/**
 * Create a DOM element
 *
 * This creates and populates a DOM Node.
 *
 * \param document
 * The DOM document this node is part of
 *
 * \param name
 * A name of the form <namespace>:<name>
 *
 * \returns
 * Pointer to DOM Node
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_element(OFC_DOMNode *document,
                       const OFC_DOMString *name);
/**
 * Create a DOM CDATA Node and populates the value of the Node
 *
 * \param document
 * Document this node is part of
 *
 * \param data
 * Pointer to data string
 *
 * \returns
 * Pointer to DOM node
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_cdata_section(OFC_DOMNode *document,
                             const OFC_DOMString *data);

/**
 * Create a Processing Instruction Node
 *
 * Creates a processing node and populates the node name and value
 * as desired for the instruction
 *
 * \param document
 * Document this node is part of
 *
 * \param target
 * The target value which is populated in the nodeName field
 *
 * \param data
 * The data value which is populated in the nodeValue field
 *
 * \returns
 * Pointer to dom node
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_processing_instruction(OFC_DOMNode *document,
                                      const OFC_DOMString *target,
                                      const OFC_DOMString *data);
/**
 * Return an attribute of a DOM Node
 *
 * \param elem
 * The DOM Node to lookup the attribute in
 *
 * \param name
 * The attribute name to lookup
 *
 * \returns
 * The string representing the attribute.  This string should
 * be considered non-volotile (i.e. constant).  It should not be
 * freed or modified.
 */
OFC_CORE_LIB OFC_DOMString *
ofc_dom_get_attribute(OFC_DOMNode *elem,
                      const OFC_DOMString *name);
/**
 * Set an Attribute for a node
 *
 * If the attribute already exists, the value is freed and replaced.
 * If the attribute does not exist, a new attribute is created.
 *
 * \param elem
 * The element to add the attribute to
 *
 * \param attr
 * The attribute value
 */
OFC_CORE_LIB OFC_VOID
ofc_dom_set_attribute(OFC_DOMNode *elem,
                      const OFC_DOMString *attr,
                      const OFC_DOMString *value);
/**
 * Append a child node as the last node of an element
 *
 * There is special handling if the child node is a DOCUMENT_NODE.  In that
 * case, the grandkids of the child are removed from the child and
 * appended to the document.  The original child node is destroyed.
 *
 * \param node
 * The node to append the child to
 *
 * \param child
 * The child to append
 *
 * \returns
 * The child node.  In the case that the original child node was a DOCUMENT_NODE,
 * the child is destroyed.  The document node itself is returned.
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_append_child(OFC_DOMNode *node, OFC_DOMNode *child);
/**
 * Return a Node List of nodes with matching names
 *
 * This will construct a node list of all nodes in the ancestry of the
 * starting node which match the node name.
 *
 * \param node
 * The start of the node hierarchy to search for matching node names
 *
 * \param name
 * The name to search for
 *
 * \returns
 * A list of node pointers.  The end of the list is marked by a NULL pointer
 * The nodelist must be freed using the \ref ofc_dom_destroy_node_list call.
 */
OFC_CORE_LIB OFC_DOMNodelist *
ofc_dom_get_elements_by_tag_name(OFC_DOMNode *node,
                                 const OFC_DOMString *name);
/**
 * Search for a matching element, then get the CDATA of the match
 *
 * \param node
 * The DOM node to start searching for matching element names
 *
 * \param name
 * The element name to search for
 *
 * \returns
 * the CDATA section of the element node.
 */
OFC_CORE_LIB OFC_CHAR *
ofc_dom_get_element_cdata(OFC_DOMNode *doc, OFC_CHAR *name);
/**
 * Search for a matching element, then get the unescaped CDATA of the match
 *
 * Normally, CDATA stored in a node may be escaped so that special characters
 * conform to valid XML.  This call will unescape such CDATA.
 *
 * \param node
 * The DOM node to start searching for matching element names
 *
 * \param name
 * The element name to search for
 *
 * \returns
 * the unescaped CDATA section of the element node.
 */
OFC_CORE_LIB OFC_CHAR *
ofc_dom_get_element_cdata_unescape(OFC_DOMNode *doc, OFC_CHAR *name);
/**
 * Get the CDATA of a node (no search)
 *
 * \param node
 * The node to get the CDATA of
 *
 * \returns
 * the CDATA section of the element node.
 */
OFC_CORE_LIB OFC_CHAR *
ofc_dom_get_cdata(OFC_DOMNode *node);
/**
 * Search for a matching element, then return the CDATA converted to a LONG.
 *
 * \param node
 * The DOM node to start searching for matching element names
 *
 * \param name
 * The element name to search for
 *
 * \returns
 * An unsigned long value of the CDATA
 */
OFC_CORE_LIB OFC_ULONG
ofc_dom_get_element_cdata_ulong(OFC_DOMNode *doc, OFC_CHAR *name);
/**
 * Get the first matching element in a node hierarchy
 *
 * \param doc
 * Start of node hierarchy
 *
 * \param name
 * Name of element to search for
 *
 * \returns
 * Pointer to node of matching element
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_get_element(OFC_DOMNode *doc, const OFC_CHAR *name);
/**
 * Creates a CDATA section element
 *
 * \param doc
 * The document to create the nodes within
 *
 * \param name
 * The name of the element to create
 *
 * \param string
 * The string to populate the CDATA with.  The string needs to be
 * escaped if necessary prior.
 *
 * \returns
 * The element node created
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_element_cdata(OFC_DOMNode *doc, OFC_CHAR *name,
                             const OFC_CHAR *string);
/**
 * Destroy a node list
 *
 * Destroys a node list created by a call to 
 * \ref ofc_dom_get_elements_by_tag_name
 *
 * \param nodelist
 * Pointer to the nodelist
 */
OFC_CORE_LIB OFC_VOID
ofc_dom_destroy_node_list(OFC_DOMNodelist *nodelist);
/**
 * Destroy a dom document
 *
 * Destroys all nodes in a document.
 *
 * \param node
 * This should be the document node although it can be any DOM node.
 */
OFC_CORE_LIB OFC_VOID
ofc_dom_destroy_document(OFC_DOMNode *node);
/**
 * Destroy a single DOM node
 *
 * \param node
 * The node to destroy
 */
OFC_CORE_LIB OFC_VOID
ofc_dom_destroy_node(OFC_DOMNode *node);
/**
 * Print the XML associated with a node into a buffer
 *
 * \param buf
 * Pointer to a buffer to output the XML into
 *
 * \param len
 * Length of the buffer
 *
 * \param node
 * Node to print
 *
 * \param level
 * Indentation level of the print
 *
 * \returns
 * Number of characters added to the buffer
 */
OFC_CORE_LIB OFC_SIZET
ofc_dom_sprint_node(OFC_CHAR *buf, OFC_SIZET len,
                    OFC_DOMNode *node, OFC_INT level);
/**
 * Print the XML associated with an entire document
 *
 * \param buf
 * Pointer to a buffer to output the XML into
 *
 * \param len
 * Length of the buffer
 *
 * \param node
 * The document node to print
 *
 * \returns
 * Number of characters added to the buffer
 */
OFC_CORE_LIB OFC_SIZET
ofc_dom_sprint_document(OFC_CHAR *buf, OFC_SIZET len,
                        OFC_DOMNode *node);

/**
 * Callback for reading raw DOM document
 *
 * Used by the \ref ofc_dom_load_document routine.
 *
 * \param context
 * The context passed into the load_document routine
 *
 * \param buf
 * A pointer to the buffer to read into
 *
 * \param size
 * The size of the buffer
 *
 * \returns
 * The number of bytes read
 */
typedef OFC_SIZET (*ofc_dom_load_callback)(OFC_VOID *context,
					   OFC_LPVOID buf,
					   OFC_DWORD size);
/**
 * Load a document from an external source
 *
 * \param callback
 * A callback to retrieve the raw XML document from.  See
 * \ref ofc_dom_load_callback.  If reading from
 * a file, the file should be opened/closed outside of this function.  
 * Each callback should read a buffer from the file.
 *
 * \param context
 * A context to be passed to the callback (eg. file handle)
 *
 * \returns
 * A dom document
 */
OFC_CORE_LIB OFC_DOMNode *
ofc_dom_load_document(ofc_dom_load_callback callback,
                      OFC_VOID *context);
/**
 * Unescape a character string
 *
 * \param data
 * String to be unescaped.  Unescaped data will be returned in place.
 *
 * \returns
 * Size of the resulting string
 */
OFC_CORE_LIB OFC_SIZET
ofc_dom_unescape(OFC_CHAR *data);

#if defined(__cplusplus)
}
#endif

/** \} */
#endif
