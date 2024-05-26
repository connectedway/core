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
#include "ofc/sax.h"

typedef enum {
    XML_IDLE,
    XML_STARTTAG,
    XML_ENDTAG,
    XML_CHARDATA,
    XML_XMLTAG,
    XML_OPENTAG,
    XML_COMMENTTAG
} XML_STATE;

#define MAX_ATTS 10

typedef struct {
    XML_STATE state;
    OFC_VOID *userdata;
    STARTHANDLER *startelement;
    ENDHANDLER *endelement;
    CHARHANDLER *charhandler;
    XMLHANDLER *xmlhandler;
    OFC_CHAR tag[200];
    OFC_CHAR *tagp;
    OFC_CHAR *atts[MAX_ATTS * 2];
    OFC_CHAR *charp;
    OFC_CHAR *version;
    OFC_CHAR *encoding;
    OFC_INT standalone;
} XML_PARSER_CONTEXT;

OFC_CORE_LIB OFC_XML_PARSER
ofc_xml_parser_create(OFC_VOID *p) {
    XML_PARSER_CONTEXT *parser;

    parser = (XML_PARSER_CONTEXT *) ofc_malloc(sizeof(XML_PARSER_CONTEXT));
    parser->startelement = OFC_NULL;
    parser->endelement = OFC_NULL;
    parser->charhandler = OFC_NULL;
    parser->xmlhandler = OFC_NULL;

    parser->state = XML_IDLE;
    return ((OFC_VOID *) parser);
}

OFC_CORE_LIB OFC_VOID
ofc_xml_parser_free(OFC_VOID *parsertoken) {
    XML_PARSER_CONTEXT *parser;

    parser = (XML_PARSER_CONTEXT *) parsertoken;
    ofc_free(parser);
}

OFC_CORE_LIB OFC_VOID
ofc_xml_set_user_data(OFC_VOID *parsertoken, OFC_VOID *state) {
    XML_PARSER_CONTEXT *parser;

    parser = (XML_PARSER_CONTEXT *) parsertoken;
    parser->userdata = state;
}

OFC_CORE_LIB OFC_VOID
ofc_xml_set_element_handler(OFC_VOID *parsertoken,
                            STARTHANDLER startelement,
                            ENDHANDLER endelement) {
    XML_PARSER_CONTEXT *parser;

    parser = (XML_PARSER_CONTEXT *) parsertoken;
    parser->startelement = startelement;
    parser->endelement = endelement;
}

OFC_CORE_LIB OFC_VOID
ofc_xml_set_character_data_handler(OFC_VOID *parsertoken,
                                   CHARHANDLER chardata) {
    XML_PARSER_CONTEXT *parser;

    parser = (XML_PARSER_CONTEXT *) parsertoken;
    parser->charhandler = chardata;
}

OFC_CORE_LIB OFC_VOID
ofc_xml_set_xml_decl_handler(OFC_VOID *parsertoken, XMLHANDLER xmldata) {
    XML_PARSER_CONTEXT *parser;

    parser = (XML_PARSER_CONTEXT *) parsertoken;
    parser->xmlhandler = xmldata;
}

static OFC_VOID
XML_ParseTagXML(XML_PARSER_CONTEXT *parser) {
    OFC_INT i;
    OFC_CHAR *p;

    p = parser->tag;
    while ((*p != ' ') && (*p != '\t') && (*p != '\n') && (*p != '\0'))
        p++;
    if (*p != '\0')
        *p++ = '\0';

    while ((*p == ' ') || (*p == '\t') || (*p == '\n'))
        p++;

    i = 0;
    parser->atts[i] = OFC_NULL;
    parser->atts[i + 1] = OFC_NULL;

    while ((*p != '\0') && (i < MAX_ATTS * 2)) {
        parser->atts[i] = p;
        while ((*p != ' ') && (*p != '\t') &&
               (*p != '\n') && (*p != '\0') &&
               (*p != '='))
            p++;

        if (*p == '=') {
            *p++ = '\0';

            while ((*p == ' ') || (*p == '\t') ||
                   (*p == '\n'))
                p++;
            parser->atts[i + 1] = p;

            if (*p == '"') {
                p++;
                parser->atts[i + 1] = p;
                while ((*p != '"') && (*p != '\0'))
                    p++;
            } else {
                while ((*p != ' ') && (*p != '\t') &&
                       (*p != '\n') && (*p != '\0'))
                    p++;
            }
            if (*p != '\0')
                *p++ = '\0';
        }

        while ((*p == ' ') || (*p == '\t') ||
               (*p == '\n'))
            p++;

        i += 2;
        parser->atts[i] = OFC_NULL;
        parser->atts[i + 1] = OFC_NULL;
    }

    parser->version = OFC_NULL;
    parser->encoding = OFC_NULL;
    for (i = 0; parser->atts[i] != OFC_NULL; i += 2) {
        if (ofc_strcmp(parser->atts[i], "version") == 0)
            parser->version = parser->atts[i + 1];
        if (ofc_strcmp(parser->atts[i], "encoding") == 0)
            parser->encoding = parser->atts[i + 1];
    }
}

static OFC_VOID
XML_ParseTagAtts(XML_PARSER_CONTEXT *parser) {
    OFC_INT i;
    OFC_CHAR *p;

    p = parser->tag;
    while ((*p != ' ') && (*p != '\t') && (*p != '\n') && (*p != '\0'))
        p++;
    if (*p != '\0')
        *p++ = '\0';

    while ((*p == ' ') || (*p == '\t') || (*p == '\n'))
        p++;

    i = 0;
    parser->atts[i] = OFC_NULL;
    parser->atts[i + 1] = OFC_NULL;

    while ((*p != '\0') && (i < MAX_ATTS * 2)) {
        parser->atts[i] = p;
        while ((*p != ' ') && (*p != '\t') &&
               (*p != '\n') && (*p != '\0') &&
               (*p != '='))
            p++;

        if (*p == '=') {
            *p++ = '\0';
            while ((*p == ' ') || (*p == '\t') ||
                   (*p == '\n'))
                p++;
            parser->atts[i + 1] = p;
            if (*p == '"') {
                p++;
                parser->atts[i + 1] = p;
                while ((*p != '"') && (*p != '\0'))
                    p++;
            } else {
                while ((*p != ' ') && (*p != '\t') &&
                       (*p != '\n') && (*p != '\0'))
                    p++;
            }
            if (*p != '\0')
                *p++ = '\0';
        }

        while ((*p == ' ') || (*p == '\t') ||
               (*p == '\n'))
            p++;

        i += 2;
        parser->atts[i] = OFC_NULL;
        parser->atts[i + 1] = OFC_NULL;
    }
}

OFC_CORE_LIB OFC_INT
ofc_xml_parse(OFC_VOID *parsertoken, OFC_CHAR *buf,
              OFC_SIZET len, OFC_INT done) {
    XML_PARSER_CONTEXT *parser;
    OFC_CHAR *p;

    parser = (XML_PARSER_CONTEXT *) parsertoken;
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
    p = buf;
    parser->charp = p;

    while ((len > 0) && (*p != '\0')) {
        switch (*p) {
            case '/':
                if ((parser->state == XML_STARTTAG) &&
                    (*(p + 1) == '>')) {
                    /*
                     * Call the start tag handler
                     */
                    *(parser->tagp) = '\0';
                    XML_ParseTagAtts(parser);

                    if (parser->startelement != OFC_NULL)
                        (*parser->startelement)(parser->userdata,
                                                parser->tag,
                                                (OFC_CCHAR **) &(parser->atts));
                    parser->state = XML_ENDTAG;
                } else if (parser->state == XML_OPENTAG) {
                    parser->state = XML_ENDTAG;
                } else if ((parser->state == XML_XMLTAG) ||
                           (parser->state == XML_ENDTAG))
                    *(parser->tagp++) = *p;
                break;
            case '<':
                if (parser->state == XML_IDLE) {
                    if (parser->charp != p) {
                        if (parser->charhandler != OFC_NULL)
                            (*parser->charhandler)(parser->userdata,
                                                   parser->charp,
                                                   (OFC_INT)
                                                           ((OFC_DWORD_PTR) p -
                                                            (OFC_DWORD_PTR) parser->charp));
                    }
                    parser->state = XML_OPENTAG;
                    parser->tagp = parser->tag;
                }
                break;
            case '?':
                if (parser->state == XML_OPENTAG) {
                    parser->state = XML_XMLTAG;
                }
                break;
            case '!':
                if (parser->state == XML_OPENTAG) {
                    parser->state = XML_COMMENTTAG;
                }
                break;
            case '>':
                if (parser->state == XML_STARTTAG) {
                    *(parser->tagp) = '\0';
                    XML_ParseTagAtts(parser);
                    if (parser->startelement != OFC_NULL)
                        (*parser->startelement)(parser->userdata,
                                                parser->tag,
                                                (OFC_CCHAR **) &(parser->atts));
                    parser->state = XML_IDLE;
                    parser->charp = p + 1;
                } else if (parser->state == XML_ENDTAG) {
                    *(parser->tagp) = '\0';
                    XML_ParseTagAtts(parser);
                    if (parser->endelement != OFC_NULL)
                        (*parser->endelement)(parser->userdata,
                                              parser->tag);
                    parser->state = XML_IDLE;
                    parser->charp = p + 1;
                } else if (parser->state == XML_XMLTAG) {
                    *(parser->tagp) = '\0';
                    XML_ParseTagXML(parser);
                    if (parser->xmlhandler != OFC_NULL)
                        (*parser->xmlhandler)(parser->userdata,
                                              parser->version,
                                              parser->encoding,
                                              parser->standalone);
                    parser->state = XML_IDLE;
                    parser->charp = p + 1;
                } else if (parser->state == XML_COMMENTTAG) {
                    parser->state = XML_IDLE;
                    parser->charp = p + 1;
                }
                break;


            default:
                if (parser->state == XML_OPENTAG) {
                    parser->state = XML_STARTTAG;
                    *(parser->tagp++) = *p;
                } else if ((parser->state == XML_STARTTAG) ||
                           (parser->state == XML_ENDTAG) ||
                           (parser->state == XML_XMLTAG))
                    *(parser->tagp++) = *p;
                break;
        }
        p++;
        len--;
    }

    if (len == 0) {
        if ((parser->state == XML_IDLE) && (parser->charp != p)) {
            if (parser->charhandler != OFC_NULL)
                (*parser->charhandler)(parser->userdata,
                                       parser->charp,
                                       (OFC_INT)
                                               ((OFC_DWORD_PTR) p -
                                                (OFC_DWORD_PTR) parser->charp));
        }
    }

    return (0);
}
