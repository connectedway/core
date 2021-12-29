/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#if !defined(__OFC_SAX_H__)
#define __OFC_SAX_H__

#include "ofc/core.h"
#include "ofc/types.h"
#include "ofc/persist.h"

/**
 * \defgroup Open Files SAX Parser
 *
 * \ingroup persist
 */

/** \{ */

typedef OFC_VOID (STARTHANDLER)(OFC_VOID *state,
                                OFC_CCHAR *name,
                                OFC_CCHAR **atts);

typedef OFC_VOID (ENDHANDLER)(OFC_VOID *state,
                              OFC_CCHAR *name);

typedef OFC_VOID (CHARHANDLER)(OFC_VOID *state,
                               OFC_CCHAR *str,
                               OFC_INT len);

typedef OFC_VOID (XMLHANDLER)(OFC_VOID *state,
                              OFC_CCHAR *version,
                              OFC_CCHAR *encoding,
                              OFC_INT standalone);

typedef OFC_VOID *OFC_XML_PARSER;
#define OFC_XML_STATUS_ERROR -1

#if defined(__cplusplus)
extern "C"
{
#endif
OFC_CORE_LIB OFC_XML_PARSER
ofc_xml_parser_create(OFC_VOID *p);

OFC_CORE_LIB OFC_VOID
ofc_xml_parser_free(OFC_VOID *parsertoken);

OFC_CORE_LIB OFC_VOID
ofc_xml_set_user_data(OFC_VOID *parsertoken, OFC_VOID *state);

OFC_CORE_LIB OFC_VOID
ofc_xml_set_element_handler(OFC_VOID *parsertoken,
                            STARTHANDLER startelement,
                            ENDHANDLER endelement);

OFC_CORE_LIB OFC_VOID
ofc_xml_set_character_data_handler(OFC_VOID *parsertoken,
                                   CHARHANDLER chardata);

OFC_CORE_LIB OFC_VOID
ofc_xml_set_xml_decl_handler(OFC_VOID *parsertoken,
                             XMLHANDLER xmldata);

OFC_CORE_LIB OFC_INT
ofc_xml_parse(OFC_VOID *parsertoken, OFC_CHAR *buf,
              OFC_SIZET len, OFC_INT done);

#if defined(__cplusplus)
}
#endif

#endif


/** \} */
