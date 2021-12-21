/* Copyright (c) 2009 Blue Peach Solutions, Inc.
 * All rights reserved.
 *
 * This software is protected by copyright and intellectual 
 * property laws as well as international treaties.  It is to be 
 * used and copied only by authorized licensees under the 
 * conditions described in their licenses.  
 *
 * Title to and ownership of the software shall at all times 
 * remain with Blue Peach Solutions and/or prior copyright holders.
 * 
 * Blue Peach has received permission to use and distribute the software
 * under Blue Peach's terms
 */
#if !defined(__BLUE_SAX_H__)
#define __BLUE_SAX_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/persist.h"

/**
 * \defgroup BlueSax SAX Parser
 *
 * \ingroup BlueConfig
 */

/** \{ */

typedef BLUE_VOID (STARTHANDLER)(BLUE_VOID *state, 
				 BLUE_CCHAR *name,
				 BLUE_CCHAR **atts) ;

typedef BLUE_VOID (ENDHANDLER)(BLUE_VOID *state, 
			       BLUE_CCHAR *name) ;

typedef BLUE_VOID (CHARHANDLER)(BLUE_VOID *state, 
				BLUE_CCHAR *str,
				BLUE_INT len) ;

typedef BLUE_VOID (XMLHANDLER)(BLUE_VOID *state, 
			       BLUE_CCHAR *version,
			       BLUE_CCHAR *encoding,
			       BLUE_INT standalone) ;

typedef BLUE_VOID *BLUE_XML_PARSER ;
#define BLUE_XML_STATUS_ERROR -1 

#if defined(__cplusplus)
extern "C"
{
#endif
  BLUE_CORE_LIB BLUE_XML_PARSER 
  BlueXMLparserCreate (BLUE_VOID *p) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueXMLparserFree (BLUE_VOID *parsertoken) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueXMLsetUserData (BLUE_VOID *parsertoken, BLUE_VOID *state) ;

  BLUE_CORE_LIB BLUE_VOID 
  BlueXMLsetElementHandler (BLUE_VOID *parsertoken, 
			    STARTHANDLER startelement,
			    ENDHANDLER endelement) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueXMLsetCharacterDataHandler (BLUE_VOID *parsertoken,
				  CHARHANDLER chardata) ;
  BLUE_CORE_LIB BLUE_VOID 
  BlueXMLsetXmlDeclHandler (BLUE_VOID *parsertoken,
			    XMLHANDLER xmldata) ;
  BLUE_CORE_LIB BLUE_INT 
  BlueXMLparse (BLUE_VOID *parsertoken, BLUE_CHAR *buf, 
		BLUE_SIZET len, BLUE_INT done) ;
#if defined(__cplusplus)
}
#endif

#endif


/** \} */
