/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#define __BLUE_CORE_DLL__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/sax.h"

typedef enum
  {
    XML_IDLE,
    XML_STARTTAG,
    XML_ENDTAG,
    XML_CHARDATA,
    XML_XMLTAG,
    XML_OPENTAG,
    XML_COMMENTTAG
  } XML_STATE ;

#define MAX_ATTS 10

typedef struct 
{
  XML_STATE state ;
  BLUE_VOID *userdata ;
  STARTHANDLER *startelement ;
  ENDHANDLER *endelement ;
  CHARHANDLER *charhandler ;
  XMLHANDLER *xmlhandler ;
  BLUE_CHAR tag[200] ;
  BLUE_CHAR *tagp ;
  BLUE_CHAR *atts[MAX_ATTS*2] ;
  BLUE_CHAR *charp ;
  BLUE_CHAR *version ;
  BLUE_CHAR *encoding ;
  BLUE_INT standalone ;
} XML_PARSER_CONTEXT ;

BLUE_CORE_LIB BLUE_XML_PARSER 
BlueXMLparserCreate (BLUE_VOID *p)
{
  XML_PARSER_CONTEXT *parser ;

  parser = (XML_PARSER_CONTEXT *) BlueHeapMalloc (sizeof (XML_PARSER_CONTEXT)) ;
  parser->startelement = BLUE_NULL ;
  parser->endelement = BLUE_NULL ;
  parser->charhandler = BLUE_NULL ;
  parser->xmlhandler = BLUE_NULL ;

  parser->state = XML_IDLE ;
  return ((BLUE_VOID *) parser) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueXMLparserFree (BLUE_VOID *parsertoken)
{
  XML_PARSER_CONTEXT *parser ;

  parser = (XML_PARSER_CONTEXT *) parsertoken ;
  BlueHeapFree (parser) ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueXMLsetUserData (BLUE_VOID *parsertoken, BLUE_VOID *state)
{
  XML_PARSER_CONTEXT *parser ;

  parser = (XML_PARSER_CONTEXT *) parsertoken ;
  parser->userdata = state ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueXMLsetElementHandler (BLUE_VOID *parsertoken, 
			  STARTHANDLER startelement,
			  ENDHANDLER endelement)
{
  XML_PARSER_CONTEXT *parser ;

  parser = (XML_PARSER_CONTEXT *) parsertoken ;
  parser->startelement = startelement ;
  parser->endelement = endelement ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueXMLsetCharacterDataHandler (BLUE_VOID *parsertoken,
				CHARHANDLER chardata)
{
  XML_PARSER_CONTEXT *parser ;

  parser = (XML_PARSER_CONTEXT *) parsertoken ;
  parser->charhandler = chardata ;
}

BLUE_CORE_LIB BLUE_VOID 
BlueXMLsetXmlDeclHandler (BLUE_VOID *parsertoken, XMLHANDLER xmldata)
{
  XML_PARSER_CONTEXT *parser ;

  parser = (XML_PARSER_CONTEXT *) parsertoken ;
  parser->xmlhandler = xmldata ;
}

static BLUE_VOID 
XML_ParseTagXML (XML_PARSER_CONTEXT *parser)
{
  BLUE_INT i ;
  BLUE_CHAR *p ;

  p = parser->tag ;
  while ((*p != ' ') && (*p != '\t') && (*p != '\n') && (*p != '\0'))
    p++ ;
  if (*p != '\0')
    *p++ = '\0' ;

  while ((*p == ' ') || (*p == '\t') || (*p == '\n'))
    p++ ;

  i = 0 ;
  parser->atts[i] = BLUE_NULL ;
  parser->atts[i+1] = BLUE_NULL ;

  while ((*p != '\0') && (i < MAX_ATTS*2))
    {
      parser->atts[i] = p ;
      while ((*p != ' ') && (*p != '\t') &&
	     (*p != '\n') && (*p != '\0') &&
	     (*p != '='))
	p++ ;

      if (*p == '=')
	{
	  *p++ = '\0' ;

	  while ((*p == ' ') || (*p == '\t') ||
		 (*p == '\n'))
	    p++ ;
	  parser->atts[i+1] = p ;

	  if (*p == '"')
	    {
	      p++ ;
	      parser->atts[i+1] = p ;
	      while ((*p != '"') && (*p != '\0'))
		p++ ;
	    }
	  else
	    {
	      while ((*p != ' ') && (*p != '\t') &&
		     (*p != '\n') && (*p != '\0'))
		p++ ;
	    }
	  if (*p != '\0')
	    *p++ = '\0' ;
	}

      while ((*p == ' ') || (*p == '\t') ||
	     (*p == '\n'))
	p++ ;

      i += 2 ;
      parser->atts[i] = BLUE_NULL ;
      parser->atts[i+1] = BLUE_NULL ;
    }

  parser->version = BLUE_NULL ;
  parser->encoding = BLUE_NULL ;
  for (i = 0 ; parser->atts[i] != BLUE_NULL ; i+=2)
    {
      if (BlueCstrcmp (parser->atts[i], "version") == 0)
	parser->version = parser->atts[i+1] ;
      if (BlueCstrcmp (parser->atts[i], "encoding") == 0)
	parser->encoding = parser->atts[i+1] ;
    }
}
  
static BLUE_VOID 
XML_ParseTagAtts (XML_PARSER_CONTEXT *parser)
{
  BLUE_INT i ;
  BLUE_CHAR *p ;

  p = parser->tag ;
  while ((*p != ' ') && (*p != '\t') && (*p != '\n') && (*p != '\0'))
    p++ ;
  if (*p != '\0')
    *p++ = '\0' ;

  while ((*p == ' ') || (*p == '\t') || (*p == '\n'))
    p++ ;

  i = 0 ;
  parser->atts[i] = BLUE_NULL ;
  parser->atts[i+1] = BLUE_NULL ;

  while ((*p != '\0') && (i < MAX_ATTS*2))
    {
      parser->atts[i] = p ;
      while ((*p != ' ') && (*p != '\t') &&
	     (*p != '\n') && (*p != '\0') &&
	     (*p != '='))
	p++ ;

      if (*p == '=')
	{
	  *p++ = '\0' ;
	  while ((*p == ' ') || (*p == '\t') ||
		 (*p == '\n'))
	    p++ ;
	  parser->atts[i+1] = p ;
	  if (*p == '"')
	    {
	      p++ ;
	      parser->atts[i+1] = p ;
	      while ((*p != '"') && (*p != '\0'))
		p++ ;
	    }
	  else
	    {
	      while ((*p != ' ') && (*p != '\t') &&
		     (*p != '\n') && (*p != '\0'))
		p++ ;
	    }
	  if (*p != '\0')
	    *p++ = '\0' ;
	}

      while ((*p == ' ') || (*p == '\t') ||
	     (*p == '\n'))
	p++ ;

      i += 2 ;
      parser->atts[i] = BLUE_NULL ;
      parser->atts[i+1] = BLUE_NULL ;
    }
}

BLUE_CORE_LIB BLUE_INT 
BlueXMLparse (BLUE_VOID *parsertoken, BLUE_CHAR *buf, 
	      BLUE_SIZET len, BLUE_INT done)
{
  XML_PARSER_CONTEXT *parser ;
  BLUE_CHAR *p ;

  parser = (XML_PARSER_CONTEXT *) parsertoken ;
  /*
   * What we want to do is read characters in the buffer
   *
   * if we see an unescaped '<', then we have a start of something and
   * we enter a 'tag' state

   * if we see a </ then we have an end of something and we enter a tag
   * state
   *
   * if we see a /> while we are in a tag state, then we have an end element
   * and we leave the tag state
   *
   * if we see a > while we are in a tag state, then we leave the tag state.
   *
   * if we see a <? while we are not in a tag state, then we enter an xml
   * state
   */
  p = buf ;
  parser->charp = p ;

  while ((len > 0) && (*p != '\0'))
    {
      switch (*p)
	{
	case '/':
	  if ((parser->state == XML_STARTTAG) &&
	      (*(p+1) == '>'))
	    {
	      /*
	       * Call the start tag handler
	       */
	      *(parser->tagp) = '\0' ;
	      XML_ParseTagAtts (parser) ;
	      
	      if (parser->startelement != BLUE_NULL)
		(*parser->startelement)(parser->userdata,
					parser->tag,
					(BLUE_CCHAR **) &(parser->atts)) ;
	      parser->state = XML_ENDTAG ;
	    }
	  else if (parser->state == XML_OPENTAG)
	    {
	      parser->state = XML_ENDTAG ;
	    }
	  else
	    *(parser->tagp++) = *p ;
	  break ;
	case '<':
	  if (parser->state == XML_IDLE)
	    {
	      if (parser->charp != p)
		{
		  if (parser->charhandler != BLUE_NULL)
		    (*parser->charhandler)(parser->userdata,
					   parser->charp,
					   (BLUE_INT) 
					   ((BLUE_DWORD_PTR) p - 
					    (BLUE_DWORD_PTR) parser->charp)) ;
		}
	      parser->state = XML_OPENTAG ;
	      parser->tagp = parser->tag ;
	    }
	  break ;
	case '?':
	  if (parser->state == XML_OPENTAG)
	    {
	      parser->state = XML_XMLTAG ;
	    }
	  break ;
	case '!':
	  if (parser->state == XML_OPENTAG)
	    {
	      parser->state = XML_COMMENTTAG ;
	    }
	  break ;
	case '>':
	  if (parser->state == XML_STARTTAG)
	    {
	      *(parser->tagp) = '\0' ;
	      XML_ParseTagAtts (parser) ;
	      if (parser->startelement != BLUE_NULL)
		(*parser->startelement)(parser->userdata,
					parser->tag,
					(BLUE_CCHAR **) &(parser->atts)) ;
	      parser->state = XML_IDLE ;
	      parser->charp = p+1 ;
	    }
	  else if (parser->state == XML_ENDTAG)
	    {
	      *(parser->tagp) = '\0' ;
	      XML_ParseTagAtts (parser) ;
	      if (parser->endelement != BLUE_NULL)
		(*parser->endelement)(parser->userdata,
				      parser->tag) ;
	      parser->state = XML_IDLE ;
	      parser->charp = p+1 ;
	    }
	  else if (parser->state == XML_XMLTAG)
	    {
	      *(parser->tagp) = '\0' ;
	      XML_ParseTagXML (parser) ;
	      if (parser->xmlhandler != BLUE_NULL)
		(*parser->xmlhandler)(parser->userdata,
				      parser->version,
				      parser->encoding,
				      parser->standalone) ;
	      parser->state = XML_IDLE ;
	      parser->charp = p+1 ;
	    }
	  else if (parser->state == XML_COMMENTTAG)
	    {
	      parser->state = XML_IDLE ;
	      parser->charp = p+1 ;
	    }
	  break ;
	  

	default:
	  if (parser->state == XML_OPENTAG)
	    {
	      parser->state = XML_STARTTAG ;
	      *(parser->tagp++) = *p ;
	    }
	  else if ((parser->state == XML_STARTTAG) || 
		   (parser->state == XML_ENDTAG) || 
		   (parser->state == XML_XMLTAG))
	    *(parser->tagp++) = *p ;
	  break ;
	}
      p++ ;
      len -- ;
    }

  if (len == 0)
    {
      if ((parser->state == XML_IDLE) && (parser->charp != p))
	{
	  if (parser->charhandler != BLUE_NULL)
	    (*parser->charhandler)(parser->userdata,
				   parser->charp,
				   (BLUE_INT) 
				   ((BLUE_DWORD_PTR) p - 
				    (BLUE_DWORD_PTR) parser->charp)) ;
	}
    }

  return (0) ;
}
