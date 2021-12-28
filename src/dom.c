/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __OFC_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/dom.h"
#include "ofc/sax.h"

static OFC_VOID
update_remainder (OFC_SIZET count, OFC_CHAR **p,
                  OFC_SIZET *total, OFC_SIZET *remainder) ;

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_node(OFC_VOID)
{
  OFC_DOMNode *node ;

  node = (OFC_DOMNode *) ofc_malloc (sizeof (OFC_DOMNode)) ;
  if (node != OFC_NULL)
    {
      node->nodeName = OFC_NULL ;
      node->ns = OFC_NULL ;
      node->nodeValue = OFC_NULL ;
      node->parentNode = OFC_NULL ;
      node->firstChild = OFC_NULL ;
      node->lastChild = OFC_NULL ;
      node->previousSibling = OFC_NULL ;
      node->nextSibling = OFC_NULL ;
      node->attributes = OFC_NULL ;
      node->ownerDocument = OFC_NULL ;
    }
  return (node) ;
}

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_document (OFC_DOMString *namespaceURI,
                         OFC_DOMString *qualifiedName,
                         OFC_DocumentType *doctype)
{
  OFC_DOMNode *doc ;

  doc = ofc_dom_create_node();
  if (doc != OFC_NULL)
    {
      doc->nodeType = DOCUMENT_NODE ;
      doc->ownerDocument = doc ;
      doc->ns = OFC_NULL ;
      doc->nodeName = ofc_strdup("ROOT") ;
    }
  return (doc) ;
}

static OFC_VOID
parseName (OFC_CCHAR *name, OFC_DOMNode *elem)
{
  OFC_INT i ;
  OFC_CHAR *p ;

  for (i = 0, p = (OFC_CHAR *) name; (*p != '\0') && (*p != ':') ; i++, p++) ;

  if (*p == ':')
    {
      elem->ns = ofc_malloc (i + 1) ;
      ofc_memcpy (elem->ns, (const OFC_LPVOID) name, i) ;
      elem->ns[i] = '\0' ;
      p++ ;
      elem->nodeName = ofc_strdup (p) ;
    }
  else
    {
      elem->ns = OFC_NULL ;
      elem->nodeName = ofc_strdup (name) ;
    }
}

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_element (OFC_DOMNode *document, const OFC_DOMString *name)
{
  OFC_DOMNode *elem ;

  elem = ofc_dom_create_node();
  if (elem != OFC_NULL)
    {
      elem->nodeType = ELEMENT_NODE ;
      elem->ownerDocument = document ;
      parseName (name, elem) ;
    }
  return (elem) ;
}

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_cdata_section (OFC_DOMNode *document,
                              const OFC_DOMString *data)
{
  OFC_DOMNode *cdata ;

  cdata = ofc_dom_create_node();
  if (cdata != OFC_NULL)
    {
      cdata->nodeType = CDATA_SECTION_NODE ;
      cdata->ownerDocument = document ;
      cdata->nodeValue = (OFC_DOMString *) ofc_strdup (data) ;
    }
  return (cdata) ;
}

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_processing_instruction (OFC_DOMNode *document,
                                       const OFC_DOMString *target,
                                       const OFC_DOMString *data)
{
  OFC_DOMNode *pi ;

  pi = ofc_dom_create_node();
  if (pi != OFC_NULL)
    {
      pi->nodeType = PROCESSING_INSTRUCTION_NODE ;
      pi->ownerDocument = document ;
      pi->ns = OFC_NULL ;
      pi->nodeName = (OFC_DOMString *) ofc_strdup (target) ;
      pi->nodeValue = (OFC_DOMString *) ofc_strdup (data) ;
    }
  return (pi) ;
}

OFC_CORE_LIB OFC_DOMString *
ofc_dom_get_attribute (OFC_DOMNode *elem, const OFC_DOMString *name)
{
  OFC_DOMNode *attr ;
  OFC_DOMString *value ;

  attr = elem->attributes ;

  while ((attr != OFC_NULL) && (ofc_strcmp (attr->nodeName,
                                            (OFC_CHAR *) name) != 0))
    {
      attr = attr->nextSibling ;
    }

  if (attr == OFC_NULL)
    value = OFC_NULL ;
  else
    value = attr->nodeValue ;

  return (value) ;
}

OFC_CORE_LIB OFC_VOID
ofc_dom_set_attribute (OFC_DOMNode *elem, const OFC_DOMString *name,
                       const OFC_DOMString *value)
{
  OFC_DOMNode *attr ;

  attr = elem->attributes ;

  while ((attr != OFC_NULL) && (ofc_strcmp (attr->nodeName, name) != 0))
    {
      attr = attr->nextSibling ;
    }

  if (attr == OFC_NULL)
    {
      /*
       * Need to add a new attribute
       */
      attr = ofc_dom_create_node();
      if (attr != OFC_NULL)
	{
	  attr->nodeType = ATTRIBUTE_NODE ;
	  attr->ownerDocument = elem->ownerDocument ;
	  parseName (name, attr) ;
	  attr->parentNode = elem ;
	  attr->nextSibling = elem->attributes ;
	  attr->previousSibling = OFC_NULL ;
	  elem->attributes = attr ;
	}
    }

  if (attr != OFC_NULL)
    {
      if (attr->nodeValue != OFC_NULL)
	ofc_free (attr->nodeValue) ;
      attr->nodeValue = (OFC_DOMString *) ofc_strdup (value) ;
    }
}

OFC_CORE_LIB OFC_VOID
BlueDOMunlinkChild (OFC_DOMNode *node)
{
  if (node->nextSibling == OFC_NULL)
    {
      if (node->parentNode != OFC_NULL)
	node->parentNode->lastChild = node->previousSibling ;
    }
  else
    node->nextSibling->previousSibling = node->previousSibling ;
    
  if (node->previousSibling == OFC_NULL)
    {
      if (node->parentNode != OFC_NULL)
	node->parentNode->firstChild = node->nextSibling ;
    }
  else
    node->previousSibling->nextSibling = node->nextSibling ;
}

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_append_child (OFC_DOMNode *document, OFC_DOMNode *child)
{
  OFC_DOMNode *node ;

  if (child->nodeType == DOCUMENT_NODE)
    {
      node = child->firstChild ;
      while (node != OFC_NULL)
	{
	  BlueDOMunlinkChild (node) ;
        ofc_dom_append_child(document, node) ;
	  node = child->firstChild ;
	}
      ofc_dom_destroy_node (child) ;
    }
  else
    {
      child->parentNode = document ;
      child->previousSibling = document->lastChild ;
        document->lastChild = child ;
      if (child->previousSibling == OFC_NULL)
	{
        document->firstChild = child ;
	}
      else
	{
	  child->previousSibling->nextSibling = child ;
	}
    }
  return (child) ;
}

OFC_CORE_LIB OFC_DOMNodelist *
ofc_dom_get_elements_by_tag_name (OFC_DOMNode *node,
                                  const OFC_DOMString *name)
{
  OFC_DOMNodelist *nodelist ;
  OFC_DOMNode *root ;
  OFC_INT nodecount ;

  nodecount = 0 ;
  nodelist = (OFC_DOMNodelist *) ofc_malloc (1 * sizeof (OFC_DOMNode *)) ;
  nodelist[0] = OFC_NULL ;
  root = node ;

  while (node != OFC_NULL)
    {
      if (node->nodeType == ELEMENT_NODE)
	{
	  if (ofc_strcmp (node->nodeName, (OFC_CHAR *) name) == 0)
	    {
	      nodecount++ ;
	      nodelist = ofc_realloc (nodelist,
					  (nodecount+1) * 
					  sizeof (OFC_DOMNode *)) ;
	      nodelist[nodecount-1] = node ;
	      nodelist[nodecount] = OFC_NULL ;
	    }
	}

      if (node->firstChild != OFC_NULL)
	node = node->firstChild ;
      else 
	{
	  while ((node != root) && (node->nextSibling == OFC_NULL))
	    node = node->parentNode ;
	  if (node == root)
	    node = OFC_NULL ;
	  else
	    node = node->nextSibling ;
	}
    }
  return (nodelist) ;
}

OFC_CORE_LIB OFC_VOID
ofc_dom_destroy_node_list (OFC_DOMNodelist *nodelist)
{
  ofc_free (nodelist) ;
}

static OFC_VOID
update_remainder (OFC_SIZET count, OFC_CHAR **p,
                  OFC_SIZET *total, OFC_SIZET *remainder)
{
  *total += count ;

  if (*remainder <= count)
    count = *remainder ;

  *remainder -= count ;
  *p += count ;
}

OFC_CORE_LIB OFC_SIZET
BlueDOMsprintAttributes (OFC_CHAR *buf, OFC_SIZET len, OFC_DOMNode *attr)
{
  OFC_CHAR *p ;
  OFC_SIZET rem ;
  OFC_SIZET count ;
  OFC_SIZET total ;

  rem = len ;
  p = buf ;
  total = 0 ;

  while (attr != OFC_NULL)
    {
      count = ofc_snprintf (p, rem, " ") ;
      update_remainder (count, &p, &total, &rem) ;

      if (attr->ns == OFC_NULL)
	count = ofc_snprintf(p, rem, "%s=\"%s\"", attr->nodeName,
                         attr->nodeValue) ;
      else
	count = ofc_snprintf(p, rem, "%s:%s=\"%s\"",
                         attr->ns,
                         attr->nodeName,
                         attr->nodeValue) ;
      update_remainder (count, &p, &total, &rem) ;
      attr = attr->nextSibling ;
    }
  return (total) ;
}

OFC_CORE_LIB OFC_SIZET
ofc_dom_sprint_document (OFC_CHAR *buf, OFC_SIZET len, OFC_DOMNode *node)
{
  OFC_CHAR *p ;
  OFC_SIZET rem ;
  OFC_SIZET count ;
  OFC_SIZET total ;

  rem = len ;
  p = buf ;
  total = 0 ;

  count = ofc_dom_sprint_node (p, rem, node, 0) ;
  update_remainder (count, &p, &total, &rem) ;

  if (total > 0)
    {
      count = ofc_snprintf (p, rem, "\n") ;
      update_remainder (count, &p, &total, &rem) ;
    }
  
  return (total) ;
}

OFC_CORE_LIB OFC_SIZET
ofc_dom_sprint_node (OFC_CHAR *buf, OFC_SIZET len,
                     OFC_DOMNode *node, OFC_INT level)
{
  OFC_INT i ;
  OFC_CHAR *p ;
  OFC_SIZET rem ;
  OFC_SIZET count ;
  OFC_SIZET total ;

  rem = len ;
  p = buf ;
  total = 0 ;

  while (node != OFC_NULL)
    {
      switch (node->nodeType)
	{
	case DOCUMENT_NODE:
	  /*
	   * Print the children
	   */
	  count = ofc_dom_sprint_node(p, rem, node->firstChild, level) ;
	  update_remainder (count, &p, &total, &rem) ;

	  break ;

	case PROCESSING_INSTRUCTION_NODE:
	  if (node->ns == OFC_NULL)
	    count = ofc_snprintf (p, rem, "<?%s %s?>\n",
                              node->nodeName, node->nodeValue) ;
	  else
	    count = ofc_snprintf (p, rem, "<?%s:%s %s?>\n",
                              node->ns, node->nodeName, node->nodeValue) ;

	  update_remainder (count, &p, &total, &rem) ;

	  break ;

	case ELEMENT_NODE:
	  for (i = 0 ; i < level ; i ++)
	    {
	      count = ofc_snprintf (p, rem, "  ") ;
	      update_remainder (count, &p, &total, &rem) ;
	    }
	      
	  if (node->ns == OFC_NULL)
	    count = ofc_snprintf (p, rem, "<%s", node->nodeName) ;
	  else
	    count = ofc_snprintf (p, rem, "<%s:%s",
                              node->ns,
                              node->nodeName) ;
	  update_remainder (count, &p, &total, &rem) ;

	  if (node->attributes != OFC_NULL)
	    {
	      count = BlueDOMsprintAttributes (p, rem, node->attributes) ;
	      update_remainder (count, &p, &total, &rem) ;
	    }
	      
	  count = ofc_snprintf(p, rem, ">") ;
	  update_remainder (count, &p, &total, &rem) ;

	  if (node->firstChild)
	    {
	      if (node->firstChild->nodeType == CDATA_SECTION_NODE)
		{
		  count = ofc_dom_sprint_node(p, rem, node->firstChild,
					    level+1) ;
		  update_remainder (count, &p, &total, &rem) ;
		}
	      else
		{
		  count = ofc_snprintf (p, rem, "\n") ;
		  update_remainder (count, &p, &total, &rem) ;

		  count = ofc_dom_sprint_node(p, rem, node->firstChild,
					    level+1) ;
		  update_remainder (count, &p, &total, &rem) ;

		  for (i = 0 ; i < level ; i ++)
		    {
		      count = ofc_snprintf (p, rem, "  ") ;
		      update_remainder (count, &p, &total, &rem) ;
		    }
		}
	    }

	  if (node->ns == OFC_NULL)
	    count = ofc_snprintf (p, rem, "</%s>\n", node->nodeName) ;
	  else
	    count = ofc_snprintf (p, rem, "</%s:%s>\n",
                              node->ns, node->nodeName) ;
	  update_remainder (count, &p, &total, &rem) ;

	  break ;

	case CDATA_SECTION_NODE:

	  count = ofc_snprintf (p, rem, "%s", node->nodeValue) ;
	  update_remainder (count, &p, &total, &rem) ;

	  break ;
	  
	default:
	  break ;

	}

      node = node->nextSibling ;
      while (node && (node->nodeType == CDATA_SECTION_NODE))
	{
	  node = node->nextSibling ;
	}

    }

  return (total) ;
}

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_create_element_cdata (OFC_DOMNode *doc, OFC_CHAR *name,
                              const OFC_CHAR *string)
{
  OFC_DOMNode *node ;
  OFC_DOMNode *cdata ;

  node = ofc_dom_create_element(doc, name) ;
  if ((node != OFC_NULL) &&
      ((string != OFC_NULL) && (string[0] != '\0')))
    {
      cdata = ofc_dom_create_cdata_section(doc, string) ;
      if (cdata != OFC_NULL)
          ofc_dom_append_child(node, cdata) ;
      else
	{
	  ofc_dom_destroy_document (node) ;
	  node = OFC_NULL ;
	}
    }
  return (node) ;
}

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_get_element (OFC_DOMNode *doc, const OFC_CHAR *name)
{
  OFC_BOOL found ;
  OFC_DOMNode *child ;
  OFC_DOMNode *node ;

  child = doc->firstChild ;
  found = OFC_FALSE ;

  while ((child != OFC_NULL) && !found)
    {
      if ((child->nodeType == ELEMENT_NODE) &&
	  (ofc_strcmp (child->nodeName, name) == 0))
	found = 1 ;
      else
	child = child->nextSibling ;
    }

  if (!found)
    {
      node = doc->firstChild ;
      child = OFC_NULL ;
      while ((node != OFC_NULL) && (child == OFC_NULL))
	{
	  child = ofc_dom_get_element (node, name) ;
	  if (child == OFC_NULL)
	    node = node->nextSibling ;
	}
    }

  return (child) ;
}

OFC_CORE_LIB OFC_CHAR *
ofc_dom_get_cdata (OFC_DOMNode *node)
{
  OFC_CHAR *text ;

  if ((node->firstChild != OFC_NULL) &&
      (node->firstChild->nodeType == CDATA_SECTION_NODE))
    text = node->firstChild->nodeValue ;
  else
    text = OFC_NULL ;

  return (text) ;
} 

OFC_CORE_LIB OFC_CHAR *
ofc_dom_get_element_cdata (OFC_DOMNode *doc, OFC_CHAR *name)
{
  OFC_CHAR *text ;
  OFC_DOMNode *node ;

  text = OFC_NULL ;
  node = ofc_dom_get_element (doc, name) ;
  if (node != OFC_NULL)
    {
      text = ofc_dom_get_cdata(node) ;
    }
  return (text) ;
}

OFC_CORE_LIB OFC_CHAR *
ofc_dom_get_element_cdata_unescape (OFC_DOMNode *doc, OFC_CHAR *name)
{
  OFC_CHAR *cdata ;

  cdata = ofc_dom_get_element_cdata(doc, name) ;

  if (cdata != OFC_NULL)
    ofc_dom_unescape (cdata) ;

  return (cdata) ;
}

OFC_CORE_LIB OFC_SIZET
ofc_dom_unescape (OFC_CHAR* data)
{
  OFC_CHAR *end = data + ofc_strlen(data);
  OFC_CHAR *i = data;
  OFC_CHAR *j = data;

  /*
   * Iterate through the string to find escaped sequences
   */
  while (j < end)
    {
      if (j[0] == '&' && j[1] == 'q' && j[2] == 'u' &&
	  j[3] == 'o' && j[4] == 't' && j[5] == ';')
	{
	  /* Double Quote */
	  i[0] = '"';
	  j += 5;
	}
      else if (j[0] == '&' && j[1] == 'a' && j[2] == 'p' &&
	       j[3] == 'o' && j[4] == 's' && j[5] == ';')
	{
	  /* Single Quote (apostrophe) */
	  i[0] = '\'';
	  j += 5;
	}
      else if (j[0] == '&' && j[1] == 'a' && j[2] == 'm' &&
	       j[3] == 'p' && j[4] == ';')
	{
	  /* Ampersand */
	  i[0] = '&';
	  j += 4;
	}
      else if (j[0] == '&' && j[1] == 'l' && j[2] == 't' &&
	       j[3] == ';')
	{
	  /* Less Than */
	  i[0] = '<';
	  j += 3;
	}
      else if (j[0] == '&' && j[1] == 'g' && j[2] == 't' &&
	       j[3] == ';')
	{
	  /* Greater Than */
	  i[0] = '>';
	  j += 3;
	}
      else
	{
	  i[0] = j[0];
	}
      i++;
      j++;
    }
  i[0] = '\0';

  return (OFC_SIZET)(i - data);
}

OFC_CORE_LIB OFC_ULONG
ofc_dom_get_element_cdata_long (OFC_DOMNode *doc, OFC_CHAR *name)
{
  OFC_CHAR *cdata ;
  OFC_ULONG val ;

  cdata = ofc_dom_get_element_cdata(doc, name) ;

  if (cdata != OFC_NULL)
    val = ofc_strtoul (cdata, OFC_NULL, 0) ;
  else
    val = 0 ;

  return (val) ;
}

OFC_CORE_LIB OFC_VOID
ofc_dom_destroy_node (OFC_DOMNode *node)
{
  if (node->ns != OFC_NULL)
    ofc_free (node->ns) ;
  if (node->nodeName != OFC_NULL)
    ofc_free (node->nodeName) ;
  if (node->nodeValue != OFC_NULL)
    ofc_free (node->nodeValue) ;
  ofc_free (node) ;
}

OFC_CORE_LIB OFC_VOID
ofc_dom_destroy_document (OFC_DOMNode *node)
{
  OFC_DOMNode *attr ;

  while (node->firstChild != OFC_NULL)
    ofc_dom_destroy_document (node->firstChild) ;

  while (node->attributes != OFC_NULL)
    {
      attr = node->attributes ;
      node->attributes = node->attributes->nextSibling ;
      ofc_dom_destroy_node (attr) ;
    }

  /*
   * Unlink this guy from the node list
   */
  BlueDOMunlinkChild (node) ;

  ofc_dom_destroy_node (node) ;
}

typedef struct dom_state
{
  OFC_INT depth ;
  OFC_DOMNode *currentNode ;
  OFC_SIZET valuelen ;
  OFC_CHAR *value ;
  OFC_DOMNode *document ;
  BLUE_XML_PARSER parser ; 
} DOMState ;

static OFC_VOID
startElement(OFC_VOID *userData, OFC_CCHAR *name, OFC_CCHAR **atts)
{
  OFC_DOMNode *elem ;
  DOMState *dom_state ;

  dom_state = (DOMState *) userData ;

  dom_state->depth += 1 ;

  elem = ofc_dom_create_element(dom_state->document, name) ;

  if (elem != OFC_NULL)
    {
        ofc_dom_append_child(dom_state->currentNode, elem) ;
      dom_state->currentNode = elem ;

      /*
       * Check attributes
       */
      while ((atts != OFC_NULL) && (*atts != OFC_NULL))
	{
        ofc_dom_set_attribute(elem, *(atts), *(atts + 1)) ;
	  atts+=2 ;
	}
    }

  if (dom_state->value != OFC_NULL)
    {
      ofc_free (dom_state->value) ;
      dom_state->value = OFC_NULL ;
    }
  dom_state->valuelen = 0 ;
}

static OFC_VOID
endElement(OFC_VOID *userData, OFC_CCHAR *name)
{
  DOMState *dom_state ;
  OFC_DOMNode *elem ;
  OFC_DOMNode *cdata ;

  dom_state = (DOMState *) userData ;

  dom_state->depth -= 1 ;
  elem = dom_state->currentNode ;

  if (dom_state->value != OFC_NULL)
    {
      cdata = ofc_dom_create_cdata_section(dom_state->document,
                                           dom_state->value) ;
        ofc_dom_append_child(elem, cdata) ;

      ofc_free (dom_state->value) ;
      dom_state->valuelen = 0 ;
      dom_state->value = OFC_NULL ;
    }

  dom_state->currentNode = elem->parentNode ;
}

static OFC_VOID
characterData(OFC_VOID *userData, OFC_CCHAR *str, OFC_INT len)
{
  DOMState *dom_state ;

  dom_state = (DOMState *) userData ;

  /*
   * strip leading white space
   */
  if (dom_state->value == OFC_NULL)
    {
      for ( ;
	   ((len > 0) && 
	    ((*str == ' ') ||
	     (*str == '\n') ||
	     (*str == '\t'))); 
	   len--, str++)
	/* empty */ ;
    }

  if (len > 0)
    {
      dom_state->value = 
	(OFC_CHAR *) ofc_realloc (dom_state->value,
				       dom_state->valuelen+len+1) ;
      ofc_strncpy (dom_state->value + dom_state->valuelen, str, len) ;
      dom_state->valuelen += len ;
      dom_state->value[dom_state->valuelen] = '\0' ;
    }
}

static OFC_VOID
xmlDeclaration(OFC_VOID *userData, OFC_CCHAR *version,
               OFC_CCHAR *encoding, OFC_INT standalone)
{
  OFC_DOMNode *pi ;
  DOMState *dom_state ;
  OFC_CHAR *data ;

  OFC_CHAR *p ;
  OFC_SIZET rem ;
  OFC_SIZET count ;
  OFC_SIZET total ;

  dom_state = (DOMState *) userData ;
  data = OFC_NULL ;

  for (total = 0, p = OFC_NULL ; p == OFC_NULL ; )
    {
      if (total == 0)
	{
	  rem = 0 ;
	}
      else
	{
	  data = (OFC_CHAR *) ofc_malloc (total + 1) ;
	  p = data ;
	  rem = total + 1 ;
	}

      if (version != OFC_NULL)
	{
	  count = ofc_snprintf (p, rem, "version=\"%s\" ", version) ;
	  update_remainder (count, &p, &total, &rem) ;
	}

      if (encoding != OFC_NULL)
	{
	  count = ofc_snprintf (p, rem, "encoding=\"%s\" ", encoding) ;
	  update_remainder (count, &p, &total, &rem) ;
	}

      count = ofc_snprintf (p, rem, "standalone = \"%s\"",
			     standalone == 0 ? "no" : 
			     standalone == 1 ? "yes" : 
			     "") ;
      update_remainder (count, &p, &total, &rem) ;
    }

  if (data != OFC_NULL)
    {
      pi = ofc_dom_create_processing_instruction(dom_state->document,
                                                 "xml", data) ;
      ofc_free (data) ;

      if (pi != OFC_NULL)
	{
        ofc_dom_append_child(dom_state->currentNode, pi) ;
	}
    }
}

OFC_CORE_LIB OFC_DOMNode *
ofc_dom_load_document (OFC_SIZET callback(OFC_VOID*, OFC_LPVOID, OFC_DWORD),
                       OFC_VOID *context)
{
  OFC_DOMNode *doc ;
  DOMState *dom_state ;
  OFC_CHAR *buf ;
  OFC_SIZET len ;
  OFC_BOOL done ;

  doc = OFC_NULL ;
  dom_state = (DOMState *) ofc_malloc (sizeof (DOMState)) ;
  if (dom_state != OFC_NULL)
    {
      dom_state->value = OFC_NULL ;
      dom_state->valuelen = 0 ;
      dom_state->depth = 0 ;
      doc = ofc_dom_create_document(OFC_NULL, OFC_NULL, OFC_NULL) ;
      if (doc != OFC_NULL)
	{
	  dom_state->document = doc ;

	  dom_state->currentNode = doc ;

	  dom_state->parser = BlueXMLparserCreate(OFC_NULL) ;

	  if (dom_state->parser != OFC_NULL)
	    {
	      BlueXMLsetUserData(dom_state->parser, dom_state);
	      BlueXMLsetElementHandler(dom_state->parser, 
				       startElement, endElement);
	      BlueXMLsetCharacterDataHandler (dom_state->parser, 
					      characterData) ;
	      BlueXMLsetXmlDeclHandler (dom_state->parser, xmlDeclaration) ;

	      buf = (OFC_CHAR *) ofc_malloc (1024) ;

	      done = 0 ;
	      while (done == OFC_FALSE)
		{
		  len = (*callback)(context, buf, 1024) ;

		  if (len == 0)
		    done = 1 ;

		  if (len >= 0)
		    {
		      if (BlueXMLparse(dom_state->parser, buf, len, done) == 
			  BLUE_XML_STATUS_ERROR) 
			{
			  doc = OFC_NULL ;
			  done = 1 ;
			}
		    }
		  else
		    done = 1 ;
		}

	      ofc_free (buf) ;
	      BlueXMLparserFree(dom_state->parser);
	    }
	}

      if (dom_state->value != OFC_NULL)
	ofc_free (dom_state->value) ;
      dom_state->value = OFC_NULL ;

      ofc_free (dom_state) ;
    }
  return (doc) ;
}
	    
