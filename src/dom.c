/* Copyright (c) 2009 Blue Peach Solutions, Inc.
 * All rights reserved.
 *
 * This software is protected by copyright and intellectual 
 * property laws as well as international treaties.  It is to be 
 * used and copied only by authorized licensees under the 
 * conditions described in their licenses.  
 *
 * Title to and ownership of the software shall at all times 
 * remain with Blue Peach Solutions.
 */

#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/dom.h"
#include "ofc/sax.h"

static BLUE_VOID 
update_remainder (BLUE_SIZET count, BLUE_CHAR **p, 
		  BLUE_SIZET *total, BLUE_SIZET *remainder) ;

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMcreateNode (BLUE_VOID)
{
  BLUE_DOMNode *node ;

  node = (BLUE_DOMNode *) BlueHeapMalloc (sizeof (BLUE_DOMNode)) ;
  if (node != BLUE_NULL)
    {
      node->nodeName = BLUE_NULL ;
      node->ns = BLUE_NULL ;
      node->nodeValue = BLUE_NULL ;
      node->parentNode = BLUE_NULL ;
      node->firstChild = BLUE_NULL ;
      node->lastChild = BLUE_NULL ;
      node->previousSibling = BLUE_NULL ;
      node->nextSibling = BLUE_NULL ;
      node->attributes = BLUE_NULL ;
      node->ownerDocument = BLUE_NULL ;
    }
  return (node) ;
}

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMcreateDocument (BLUE_DOMString *namespaceURI, 
		       BLUE_DOMString *qualifiedName,
		       BLUE_DocumentType *doctype) 
{
  BLUE_DOMNode *doc ;

  doc = BlueDOMcreateNode() ;
  if (doc != BLUE_NULL)
    {
      doc->nodeType = DOCUMENT_NODE ;
      doc->ownerDocument = doc ;
      doc->ns = BLUE_NULL ;
      doc->nodeName = BlueCstrdup("ROOT") ;
    }
  return (doc) ;
}

static BLUE_VOID 
parseName (BLUE_CCHAR *name, BLUE_DOMNode *elem) 
{
  BLUE_INT i ;
  BLUE_CHAR *p ;

  for (i = 0, p = (BLUE_CHAR *) name; (*p != '\0') && (*p != ':') ; i++, p++) ;

  if (*p == ':')
    {
      elem->ns = BlueHeapMalloc (i+1) ;
      BlueCmemcpy (elem->ns, (const BLUE_LPVOID) name, i) ;
      elem->ns[i] = '\0' ;
      p++ ;
      elem->nodeName = BlueCstrdup (p) ;
    }
  else
    {
      elem->ns = BLUE_NULL ;
      elem->nodeName = BlueCstrdup (name) ;
    }
}

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMcreateElement (BLUE_DOMNode *document, const BLUE_DOMString *name)
{
  BLUE_DOMNode *elem ;

  elem = BlueDOMcreateNode() ;
  if (elem != BLUE_NULL)
    {
      elem->nodeType = ELEMENT_NODE ;
      elem->ownerDocument = document ;
      parseName (name, elem) ;
    }
  return (elem) ;
}

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMcreateCDATASection (BLUE_DOMNode *document, 
			   const BLUE_DOMString *data)
{
  BLUE_DOMNode *cdata ;

  cdata = BlueDOMcreateNode() ;
  if (cdata != BLUE_NULL)
    {
      cdata->nodeType = CDATA_SECTION_NODE ;
      cdata->ownerDocument = document ;
      cdata->nodeValue = (BLUE_DOMString *) BlueCstrdup (data) ;
    }
  return (cdata) ;
}

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMcreateProcessingInstruction (BLUE_DOMNode *document, 
				    const BLUE_DOMString *target,
				    const BLUE_DOMString *data) 
{
  BLUE_DOMNode *pi ;

  pi = BlueDOMcreateNode() ;
  if (pi != BLUE_NULL)
    {
      pi->nodeType = PROCESSING_INSTRUCTION_NODE ;
      pi->ownerDocument = document ;
      pi->ns = BLUE_NULL ;
      pi->nodeName = (BLUE_DOMString *) BlueCstrdup (target) ;
      pi->nodeValue = (BLUE_DOMString *) BlueCstrdup (data) ;
    }
  return (pi) ;
}

BLUE_CORE_LIB BLUE_DOMString *
BlueDOMgetAttribute (BLUE_DOMNode *elem, const BLUE_DOMString *name)
{
  BLUE_DOMNode *attr ;
  BLUE_DOMString *value ;

  attr = elem->attributes ;

  while ((attr != BLUE_NULL) && (BlueCstrcmp (attr->nodeName, 
					      (BLUE_CHAR *) name) != 0))
    {
      attr = attr->nextSibling ;
    }

  if (attr == BLUE_NULL)
    value = BLUE_NULL ;
  else
    value = attr->nodeValue ;

  return (value) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueDOMsetAttribute (BLUE_DOMNode *elem, const BLUE_DOMString *name, 
		     const BLUE_DOMString *value) 
{
  BLUE_DOMNode *attr ;

  attr = elem->attributes ;

  while ((attr != BLUE_NULL) && (BlueCstrcmp (attr->nodeName, name) != 0))
    {
      attr = attr->nextSibling ;
    }

  if (attr == BLUE_NULL)
    {
      /*
       * Need to add a new attribute
       */
      attr = BlueDOMcreateNode() ;
      if (attr != BLUE_NULL)
	{
	  attr->nodeType = ATTRIBUTE_NODE ;
	  attr->ownerDocument = elem->ownerDocument ;
	  parseName (name, attr) ;
	  attr->parentNode = elem ;
	  attr->nextSibling = elem->attributes ;
	  attr->previousSibling = BLUE_NULL ;
	  elem->attributes = attr ;
	}
    }

  if (attr != BLUE_NULL)
    {
      if (attr->nodeValue != BLUE_NULL)
	BlueHeapFree (attr->nodeValue) ;
      attr->nodeValue = (BLUE_DOMString *) BlueCstrdup (value) ;
    }
}

BLUE_CORE_LIB BLUE_VOID 
BlueDOMunlinkChild (BLUE_DOMNode *node)
{
  if (node->nextSibling == BLUE_NULL)
    {
      if (node->parentNode != BLUE_NULL)
	node->parentNode->lastChild = node->previousSibling ;
    }
  else
    node->nextSibling->previousSibling = node->previousSibling ;
    
  if (node->previousSibling == BLUE_NULL)
    {
      if (node->parentNode != BLUE_NULL)
	node->parentNode->firstChild = node->nextSibling ;
    }
  else
    node->previousSibling->nextSibling = node->nextSibling ;
}

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMappendChild (BLUE_DOMNode *parent, BLUE_DOMNode *child) 
{
  BLUE_DOMNode *node ;

  if (child->nodeType == DOCUMENT_NODE)
    {
      node = child->firstChild ;
      while (node != BLUE_NULL)
	{
	  BlueDOMunlinkChild (node) ;
	  BlueDOMappendChild (parent, node) ;
	  node = child->firstChild ;
	}
      BlueDOMdestroyNode (child) ;
    }
  else
    {
      child->parentNode = parent ;
      child->previousSibling = parent->lastChild ;
      parent->lastChild = child ;
      if (child->previousSibling == BLUE_NULL)
	{
	  parent->firstChild = child ;
	}
      else
	{
	  child->previousSibling->nextSibling = child ;
	}
    }
  return (child) ;
}

BLUE_CORE_LIB BLUE_DOMNodelist *
BlueDOMgetElementsByTagName (BLUE_DOMNode *node, 
			     const BLUE_DOMString *name)
{
  BLUE_DOMNodelist *nodelist ;
  BLUE_DOMNode *root ;
  BLUE_INT nodecount ;

  nodecount = 0 ;
  nodelist = (BLUE_DOMNodelist *) BlueHeapMalloc (1 * sizeof (BLUE_DOMNode *)) ;
  nodelist[0] = BLUE_NULL ;
  root = node ;

  while (node != BLUE_NULL)
    {
      if (node->nodeType == ELEMENT_NODE)
	{
	  if (BlueCstrcmp (node->nodeName, (BLUE_CHAR *) name) == 0)
	    {
	      nodecount++ ;
	      nodelist = BlueHeapRealloc (nodelist, 
					  (nodecount+1) * 
					  sizeof (BLUE_DOMNode *)) ;
	      nodelist[nodecount-1] = node ;
	      nodelist[nodecount] = BLUE_NULL ;
	    }
	}

      if (node->firstChild != BLUE_NULL)
	node = node->firstChild ;
      else 
	{
	  while ((node != root) && (node->nextSibling == BLUE_NULL))
	    node = node->parentNode ;
	  if (node == root)
	    node = BLUE_NULL ;
	  else
	    node = node->nextSibling ;
	}
    }
  return (nodelist) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueDOMdestroyNodelist (BLUE_DOMNodelist *nodelist)
{
  BlueHeapFree (nodelist) ;
}

static BLUE_VOID 
update_remainder (BLUE_SIZET count, BLUE_CHAR **p, 
		  BLUE_SIZET *total, BLUE_SIZET *remainder)
{
  *total += count ;

  if (*remainder <= count)
    count = *remainder ;

  *remainder -= count ;
  *p += count ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueDOMsprintAttributes (BLUE_CHAR *buf, BLUE_SIZET len, BLUE_DOMNode *attr)
{
  BLUE_CHAR *p ;
  BLUE_SIZET rem ;
  BLUE_SIZET count ;
  BLUE_SIZET total ;

  rem = len ;
  p = buf ;
  total = 0 ;

  while (attr != BLUE_NULL)
    {
      count = BlueCsnprintf (p, rem, " ") ;
      update_remainder (count, &p, &total, &rem) ;

      if (attr->ns == BLUE_NULL)
	count = BlueCsnprintf(p, rem, "%s=\"%s\"", attr->nodeName, 
			      attr->nodeValue) ;
      else
	count = BlueCsnprintf(p, rem, "%s:%s=\"%s\"", 
			      attr->ns,
			      attr->nodeName, 
			      attr->nodeValue) ;
      update_remainder (count, &p, &total, &rem) ;
      attr = attr->nextSibling ;
    }
  return (total) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueDOMsprintDocument (BLUE_CHAR *buf, BLUE_SIZET len, BLUE_DOMNode *node)
{
  BLUE_CHAR *p ;
  BLUE_SIZET rem ;
  BLUE_SIZET count ;
  BLUE_SIZET total ;

  rem = len ;
  p = buf ;
  total = 0 ;

  count = BlueDOMsprintNode (p, rem, node, 0) ;
  update_remainder (count, &p, &total, &rem) ;

  if (total > 0)
    {
      count = BlueCsnprintf (p, rem, "\n") ;
      update_remainder (count, &p, &total, &rem) ;
    }
  
  return (total) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueDOMsprintNode (BLUE_CHAR *buf, BLUE_SIZET len, 
		   BLUE_DOMNode *node, BLUE_INT level)
{
  BLUE_INT i ;
  BLUE_CHAR *p ;
  BLUE_SIZET rem ;
  BLUE_SIZET count ;
  BLUE_SIZET total ;

  rem = len ;
  p = buf ;
  total = 0 ;

  while (node != BLUE_NULL)
    {
      switch (node->nodeType)
	{
	case DOCUMENT_NODE:
	  /*
	   * Print the children
	   */
	  count = BlueDOMsprintNode(p, rem, node->firstChild, level) ;
	  update_remainder (count, &p, &total, &rem) ;

	  break ;

	case PROCESSING_INSTRUCTION_NODE:
	  if (node->ns == BLUE_NULL)
	    count = BlueCsnprintf (p, rem, "<?%s %s?>\n", 
				   node->nodeName, node->nodeValue) ;
	  else
	    count = BlueCsnprintf (p, rem, "<?%s:%s %s?>\n", 
				   node->ns, node->nodeName, node->nodeValue) ;

	  update_remainder (count, &p, &total, &rem) ;

	  break ;

	case ELEMENT_NODE:
	  for (i = 0 ; i < level ; i ++)
	    {
	      count = BlueCsnprintf (p, rem, "  ") ;
	      update_remainder (count, &p, &total, &rem) ;
	    }
	      
	  if (node->ns == BLUE_NULL)
	    count = BlueCsnprintf (p, rem, "<%s", node->nodeName) ;
	  else
	    count = BlueCsnprintf (p, rem, "<%s:%s", 
				   node->ns,
				   node->nodeName) ;
	  update_remainder (count, &p, &total, &rem) ;

	  if (node->attributes != BLUE_NULL)
	    {
	      count = BlueDOMsprintAttributes (p, rem, node->attributes) ;
	      update_remainder (count, &p, &total, &rem) ;
	    }
	      
	  count = BlueCsnprintf(p, rem, ">") ;
	  update_remainder (count, &p, &total, &rem) ;

	  if (node->firstChild)
	    {
	      if (node->firstChild->nodeType == CDATA_SECTION_NODE)
		{
		  count = BlueDOMsprintNode(p, rem, node->firstChild, 
					    level+1) ;
		  update_remainder (count, &p, &total, &rem) ;
		}
	      else
		{
		  count = BlueCsnprintf (p, rem, "\n") ;
		  update_remainder (count, &p, &total, &rem) ;

		  count = BlueDOMsprintNode(p, rem, node->firstChild, 
					    level+1) ;
		  update_remainder (count, &p, &total, &rem) ;

		  for (i = 0 ; i < level ; i ++)
		    {
		      count = BlueCsnprintf (p, rem, "  ") ;
		      update_remainder (count, &p, &total, &rem) ;
		    }
		}
	    }

	  if (node->ns == BLUE_NULL)
	    count = BlueCsnprintf (p, rem, "</%s>\n", node->nodeName) ;
	  else
	    count = BlueCsnprintf (p, rem, "</%s:%s>\n", 
				   node->ns, node->nodeName) ;
	  update_remainder (count, &p, &total, &rem) ;

	  break ;

	case CDATA_SECTION_NODE:

	  count = BlueCsnprintf (p, rem, "%s", node->nodeValue) ;
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

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMcreateElementCDATA (BLUE_DOMNode *doc, BLUE_CHAR *name, 
			   const BLUE_CHAR *string)
{
  BLUE_DOMNode *node ;
  BLUE_DOMNode *cdata ;

  node = BlueDOMcreateElement (doc, name) ;
  if ((node != BLUE_NULL) &&
      ((string != BLUE_NULL) && (string[0] != '\0')))
    {
      cdata = BlueDOMcreateCDATASection (doc, string) ;
      if (cdata != BLUE_NULL)
	BlueDOMappendChild (node, cdata) ;
      else
	{
	  BlueDOMdestroyDocument (node) ;
	  node = BLUE_NULL ;
	}
    }
  return (node) ;
}

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMgetElement (BLUE_DOMNode *doc, const BLUE_CHAR *name)
{
  BLUE_BOOL found ;
  BLUE_DOMNode *child ;
  BLUE_DOMNode *node ;

  child = doc->firstChild ;
  found = BLUE_FALSE ;

  while ((child != BLUE_NULL) && !found)
    {
      if ((child->nodeType == ELEMENT_NODE) &&
	  (BlueCstrcmp (child->nodeName, name) == 0))
	found = 1 ;
      else
	child = child->nextSibling ;
    }

  if (!found)
    {
      node = doc->firstChild ;
      child = BLUE_NULL ;
      while ((node != BLUE_NULL) && (child == BLUE_NULL))
	{
	  child = BlueDOMgetElement (node, name) ;
	  if (child == BLUE_NULL)
	    node = node->nextSibling ;
	}
    }

  return (child) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueDOMgetCDATA (BLUE_DOMNode *node)
{
  BLUE_CHAR *text ;

  if ((node->firstChild != BLUE_NULL) &&
      (node->firstChild->nodeType == CDATA_SECTION_NODE))
    text = node->firstChild->nodeValue ;
  else
    text = BLUE_NULL ;

  return (text) ;
} 

BLUE_CORE_LIB BLUE_CHAR *
BlueDOMgetElementCDATA (BLUE_DOMNode *doc, BLUE_CHAR *name)
{
  BLUE_CHAR *text ;
  BLUE_DOMNode *node ;

  text = BLUE_NULL ;
  node = BlueDOMgetElement (doc, name) ;
  if (node != BLUE_NULL)
    {
      text = BlueDOMgetCDATA (node) ;
    }
  return (text) ;
}

BLUE_CORE_LIB BLUE_CHAR *
BlueDOMgetElementCDATAUnescape (BLUE_DOMNode *doc, BLUE_CHAR *name)
{
  BLUE_CHAR *cdata ;

  cdata = BlueDOMgetElementCDATA (doc, name) ;

  if (cdata != BLUE_NULL)
    BlueDOMUnescape (cdata) ;

  return (cdata) ;
}

BLUE_CORE_LIB BLUE_SIZET 
BlueDOMUnescape (BLUE_CHAR* data)
{
  BLUE_CHAR *end = data + BlueCstrlen(data);
  BLUE_CHAR *i = data;
  BLUE_CHAR *j = data;

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

  return (BLUE_SIZET)(i - data);
}

BLUE_CORE_LIB BLUE_ULONG 
BlueDOMgetElementCDATAULong (BLUE_DOMNode *doc, BLUE_CHAR *name)
{
  BLUE_CHAR *cdata ;
  BLUE_ULONG val ;

  cdata = BlueDOMgetElementCDATA (doc, name) ;

  if (cdata != BLUE_NULL)
    val = BlueCstrtoul (cdata, BLUE_NULL, 0) ;
  else
    val = 0 ;

  return (val) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueDOMdestroyNode (BLUE_DOMNode *node)
{
  if (node->ns != BLUE_NULL)
    BlueHeapFree (node->ns) ;
  if (node->nodeName != BLUE_NULL)
    BlueHeapFree (node->nodeName) ;
  if (node->nodeValue != BLUE_NULL)
    BlueHeapFree (node->nodeValue) ;
  BlueHeapFree (node) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueDOMdestroyDocument (BLUE_DOMNode *node)
{
  BLUE_DOMNode *attr ;

  while (node->firstChild != BLUE_NULL)
    BlueDOMdestroyDocument (node->firstChild) ;

  while (node->attributes != BLUE_NULL)
    {
      attr = node->attributes ;
      node->attributes = node->attributes->nextSibling ;
      BlueDOMdestroyNode (attr) ;
    }

  /*
   * Unlink this guy from the node list
   */
  BlueDOMunlinkChild (node) ;

  BlueDOMdestroyNode (node) ;
}

typedef struct dom_state
{
  BLUE_INT depth ;
  BLUE_DOMNode *currentNode ;
  BLUE_SIZET valuelen ;
  BLUE_CHAR *value ;
  BLUE_DOMNode *document ;
  BLUE_XML_PARSER parser ; 
} DOMState ;

static BLUE_VOID 
startElement(BLUE_VOID *userData, BLUE_CCHAR *name, BLUE_CCHAR **atts)
{
  BLUE_DOMNode *elem ;
  DOMState *dom_state ;

  dom_state = (DOMState *) userData ;

  dom_state->depth += 1 ;

  elem = BlueDOMcreateElement (dom_state->document, name) ;

  if (elem != BLUE_NULL)
    {
      BlueDOMappendChild (dom_state->currentNode, elem) ;
      dom_state->currentNode = elem ;

      /*
       * Check attributes
       */
      while ((atts != BLUE_NULL) && (*atts != BLUE_NULL))
	{
	  BlueDOMsetAttribute (elem, *(atts), *(atts+1)) ;
	  atts+=2 ;
	}
    }

  if (dom_state->value != BLUE_NULL)
    {
      BlueHeapFree (dom_state->value) ;
      dom_state->value = BLUE_NULL ;
    }
  dom_state->valuelen = 0 ;
}

static BLUE_VOID 
endElement(BLUE_VOID *userData, BLUE_CCHAR *name)
{
  DOMState *dom_state ;
  BLUE_DOMNode *elem ;
  BLUE_DOMNode *cdata ;

  dom_state = (DOMState *) userData ;

  dom_state->depth -= 1 ;
  elem = dom_state->currentNode ;

  if (dom_state->value != BLUE_NULL)
    {
      cdata = BlueDOMcreateCDATASection (dom_state->document, 
					 dom_state->value) ;
      BlueDOMappendChild (elem, cdata) ;

      BlueHeapFree (dom_state->value) ;
      dom_state->valuelen = 0 ;
      dom_state->value = BLUE_NULL ;
    }

  dom_state->currentNode = elem->parentNode ;
}

static BLUE_VOID 
characterData(BLUE_VOID *userData, BLUE_CCHAR *str, BLUE_INT len)
{
  DOMState *dom_state ;

  dom_state = (DOMState *) userData ;

  /*
   * strip leading white space
   */
  if (dom_state->value == BLUE_NULL)
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
	(BLUE_CHAR *) BlueHeapRealloc (dom_state->value, 
				       dom_state->valuelen+len+1) ;
      BlueCstrncpy (dom_state->value + dom_state->valuelen, str, len) ;
      dom_state->valuelen += len ;
      dom_state->value[dom_state->valuelen] = '\0' ;
    }
}

static BLUE_VOID 
xmlDeclaration(BLUE_VOID *userData, BLUE_CCHAR *version,
	       BLUE_CCHAR *encoding, BLUE_INT standalone)
{
  BLUE_DOMNode *pi ;
  DOMState *dom_state ;
  BLUE_CHAR *data ;

  BLUE_CHAR *p ;
  BLUE_SIZET rem ;
  BLUE_SIZET count ;
  BLUE_SIZET total ;

  dom_state = (DOMState *) userData ;
  data = BLUE_NULL ;

  for (total = 0, p = BLUE_NULL ; p == BLUE_NULL ; )
    {
      if (total == 0)
	{
	  rem = 0 ;
	}
      else
	{
	  data = (BLUE_CHAR *) BlueHeapMalloc (total + 1) ;
	  p = data ;
	  rem = total + 1 ;
	}

      if (version != BLUE_NULL)
	{
	  count = BlueCsnprintf (p, rem, "version=\"%s\" ", version) ;
	  update_remainder (count, &p, &total, &rem) ;
	}

      if (encoding != BLUE_NULL)
	{
	  count = BlueCsnprintf (p, rem, "encoding=\"%s\" ", encoding) ;
	  update_remainder (count, &p, &total, &rem) ;
	}

      count = BlueCsnprintf (p, rem, "standalone = \"%s\"", 
			     standalone == 0 ? "no" : 
			     standalone == 1 ? "yes" : 
			     "") ;
      update_remainder (count, &p, &total, &rem) ;
    }

  if (data != BLUE_NULL)
    {
      pi = BlueDOMcreateProcessingInstruction (dom_state->document, 
					       "xml", data) ;
      BlueHeapFree (data) ;

      if (pi != BLUE_NULL)
	{
	  BlueDOMappendChild (dom_state->currentNode, pi) ;
	}
    }
}

BLUE_CORE_LIB BLUE_DOMNode *
BlueDOMloadDocument (BLUE_SIZET callback(BLUE_VOID*, BLUE_LPVOID, BLUE_DWORD), 
		     BLUE_VOID *context)
{
  BLUE_DOMNode *doc ;
  DOMState *dom_state ;
  BLUE_CHAR *buf ;
  BLUE_SIZET len ;
  BLUE_BOOL done ;

  doc = BLUE_NULL ;
  dom_state = (DOMState *) BlueHeapMalloc (sizeof (DOMState)) ;
  if (dom_state != BLUE_NULL)
    {
      dom_state->value = BLUE_NULL ;
      dom_state->valuelen = 0 ;
      dom_state->depth = 0 ;
      doc = BlueDOMcreateDocument (BLUE_NULL, BLUE_NULL, BLUE_NULL) ;
      if (doc != BLUE_NULL)
	{
	  dom_state->document = doc ;

	  dom_state->currentNode = doc ;

	  dom_state->parser = BlueXMLparserCreate(BLUE_NULL) ;

	  if (dom_state->parser != BLUE_NULL)
	    {
	      BlueXMLsetUserData(dom_state->parser, dom_state);
	      BlueXMLsetElementHandler(dom_state->parser, 
				       startElement, endElement);
	      BlueXMLsetCharacterDataHandler (dom_state->parser, 
					      characterData) ;
	      BlueXMLsetXmlDeclHandler (dom_state->parser, xmlDeclaration) ;

	      buf = (BLUE_CHAR *) BlueHeapMalloc (1024) ;

	      done = 0 ;
	      while (done == BLUE_FALSE)
		{
		  len = (*callback)(context, buf, 1024) ;

		  if (len == 0)
		    done = 1 ;

		  if (len >= 0)
		    {
		      if (BlueXMLparse(dom_state->parser, buf, len, done) == 
			  BLUE_XML_STATUS_ERROR) 
			{
			  doc = BLUE_NULL ;
			  done = 1 ;
			}
		    }
		  else
		    done = 1 ;
		}

	      BlueHeapFree (buf) ;
	      BlueXMLparserFree(dom_state->parser);
	    }
	}

      if (dom_state->value != BLUE_NULL)
	BlueHeapFree (dom_state->value) ;
      dom_state->value = BLUE_NULL ;

      BlueHeapFree (dom_state) ;
    }
  return (doc) ;
}
	    
