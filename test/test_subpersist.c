/* Copyright (c) 2021 Connected Way, LLC. All rights reserved.
 * Use of this source code is governed by a Creative Commons 
 * Attribution-NoDerivatives 4.0 International license that can be
 * found in the LICENSE file.
 */
#include "unity.h"
#include "unity_fixture.h"

#include "ofc/config.h"
#include "ofc/persist.h"
#include "ofc/dom.h"
#include "ofc/types.h"
#include "ofc/libc.h"
#include "ofc/heap.h"
#include "ofc/file.h"
#include "ofc/framework.h"
#include "ofc/env.h"

OFC_CHAR config_path[OFC_MAX_PATH+1];

static OFC_INT test_startup(OFC_VOID) {
    ofc_framework_init();
    return (0);
}

static OFC_VOID test_shutdown(OFC_VOID) {
    ofc_framework_shutdown();
    ofc_framework_destroy();
}

TEST_GROUP(subpersist);

TEST_SETUP(subpersist) {
    TEST_ASSERT_FALSE_MESSAGE(test_startup(), "Failed to Startup Framework");
}

TEST_TEAR_DOWN(subpersist) {
    test_shutdown();
}

OFC_VOID subpersist_parse(OFC_DOMNode *doc, OFC_DOMNode *sub)
{
  OFC_BOOL error_state;

  OFC_DOMNode *node;
  OFC_DOMNode *new_node;
  OFC_DOMNodelist *nodelist;
  OFC_INT i;

  node = ofc_dom_get_element(sub, "feeling");
  if (node != OFC_NULL)
    {
      nodelist = ofc_dom_get_elements_by_tag_name(node, "happy");
      if (nodelist != OFC_NULL)
	{
	  for (i = 0 ; nodelist[i] != OFC_NULL; i++)
	    {
	      new_node = nodelist[i];
	      TEST_ASSERT_FALSE_MESSAGE(ofc_strcmp(ofc_dom_get_element_cdata(new_node, "because"),
						   "it's the weekend"), "CDATA not happy");
	    }
	  TEST_ASSERT_TRUE_MESSAGE(i==1, "More than one reason to be happy");
	  ofc_dom_destroy_node_list(nodelist);
	}

      nodelist = ofc_dom_get_elements_by_tag_name(node, "sad");
      if (nodelist != OFC_NULL)
	{
	  for (i = 0 ; nodelist[i] != OFC_NULL; i++)
	    {
	      new_node = nodelist[i];
	      TEST_ASSERT_FALSE_MESSAGE(ofc_strcmp(ofc_dom_get_element_cdata(new_node, "because"),
						   "it's cold"), "CDATA not sad");
	    }
	  TEST_ASSERT_TRUE_MESSAGE(i==1, "More than one reason to be sad");
	  ofc_dom_destroy_node_list(nodelist);
	}
    }
}

OFC_BOOL subpersist_make(OFC_DOMNode *doc, OFC_DOMNode *sub)
{
  OFC_BOOL error_state;

  OFC_DOMNode *node;
  OFC_DOMNode *new_node;
  OFC_DOMNode *next_node;

  node = ofc_dom_create_element(doc, "feeling");
  if (node != OFC_NULL)
    ofc_dom_append_child(sub, node);
  else
    error_state = OFC_TRUE;

  if (!error_state)
    {
      new_node = ofc_dom_create_element(doc, "sad");
      if (new_node != OFC_NULL)
	ofc_dom_append_child(node, new_node);
      else
	error_state = OFC_TRUE;

      if (!error_state)
	{
	  next_node = ofc_dom_create_element_cdata (doc, "because", "it's cold");
	  if (next_node != OFC_NULL)
	    ofc_dom_append_child(new_node, next_node);
	  else
	    error_state = OFC_TRUE;
	}
    }

  if (!error_state)
    {
      new_node = ofc_dom_create_element(doc, "happy");
      if (new_node != OFC_NULL)
	ofc_dom_append_child(node, new_node);
      else
	error_state = OFC_TRUE;

      if (!error_state)
	{
	  next_node = ofc_dom_create_element_cdata (doc, "because", "it's the weekend");
	  if (next_node != OFC_NULL)
	    ofc_dom_append_child(new_node, next_node);
	  else
	    error_state = OFC_TRUE;
	}
    }

  return (error_state);
}


TEST(subpersist, test_subpersist_load) 
{
    OFC_BOOL ret;
    OFC_TCHAR *tpath = OFC_NULL;
    OFC_LPVOID buf;
    OFC_SIZET len;

    /*
     * We should still be registered
     */
    if (config_path[0] != '\0')
      {
	tpath = ofc_cstr2tstr(config_path);
      }
    else
      {
	tpath = ofc_malloc((OFC_MAX_PATH+1)*sizeof(OFC_TCHAR));
	if (ofc_env_get(OFC_ENV_HOME, tpath, OFC_MAX_PATH) == OFC_FALSE)
	  ret = 1;
      }

    TEST_ASSERT_FALSE_MESSAGE(ret, "Failed to get a configuration file path");

    if (ret == 0)
      {
	ret = ofc_persist_register("subpersist", subpersist_parse, subpersist_make);
	TEST_ASSERT_TRUE_MESSAGE(ret, "Failed to register subconfig");
	ofc_framework_load(tpath);

	ofc_persist_print(&buf, &len);
	TEST_ASSERT_FALSE_MESSAGE(buf==OFC_NULL, "Couldn't print dom");
	ofc_printf("After Load:\n%.*s\n", len, buf);
	ofc_free(buf);

	ret = ofc_persist_unregister("subpersist");
	TEST_ASSERT_TRUE_MESSAGE(ret, "Failed to unregister subconfig");
      }

    if (tpath != OFC_NULL)
      ofc_free(tpath);
}

TEST(subpersist, test_subpersist_init) {
    OFC_BOOL ret;
    OFC_TCHAR *tpath = OFC_NULL;
    OFC_LPVOID buf;
    OFC_SIZET len;

    if (config_path[0] != '\0')
      {
	tpath = ofc_cstr2tstr(config_path);
      }
    else
      {
	tpath = ofc_malloc((OFC_MAX_PATH+1)*sizeof(OFC_TCHAR));
	if (ofc_env_get(OFC_ENV_HOME, tpath, OFC_MAX_PATH) == OFC_FALSE)
	  ret = 1;
      }

    TEST_ASSERT_FALSE_MESSAGE(ret, "Failed to get a configuration file path");

    if (ret == 0)
      {
	/*
	 * Then register for the save
	 */
	ret = ofc_persist_register("subpersist", subpersist_parse, subpersist_make);
	TEST_ASSERT_TRUE_MESSAGE(ret, "Failed to register subconfig");
	/*
	 * Save the config with our sub
	 */
	ofc_framework_save(tpath);

	ofc_persist_print(&buf, &len);
	TEST_ASSERT_FALSE_MESSAGE(buf==OFC_NULL, "Couldn't print dom");
	ofc_printf("After Save:\n%.*s\n", len, buf);
	ofc_free(buf);

	ret = ofc_persist_unregister("subpersist");
	TEST_ASSERT_TRUE_MESSAGE(ret, "Failed to unregister subconfig");
      }
    if (tpath != OFC_NULL)
      ofc_free(tpath);
 }

TEST(subpersist, test_subpersist_save)
{
    OFC_BOOL ret;
    OFC_TCHAR *tpath = OFC_NULL;
    OFC_LPVOID buf;
    OFC_SIZET len;

    if (config_path[0] != '\0')
      {
	tpath = ofc_cstr2tstr(config_path);
      }
    else
      {
	tpath = ofc_malloc((OFC_MAX_PATH+1)*sizeof(OFC_TCHAR));
	if (ofc_env_get(OFC_ENV_HOME, tpath, OFC_MAX_PATH) == OFC_FALSE)
	  ret = 1;
      }

    TEST_ASSERT_FALSE_MESSAGE(ret, "Failed to get a configuration file path");

    if (ret == 0)
      {
	/*
	 * Save the config without our sub
	 */
	ofc_framework_save(tpath);

	ofc_persist_print(&buf, &len);
	TEST_ASSERT_FALSE_MESSAGE(buf==OFC_NULL, "Couldn't print dom");
	ofc_printf("After save:\n%.*s\n", len, buf);
	ofc_free(buf);
      }

    if (tpath != OFC_NULL)
      ofc_free(tpath);
 }

TEST_GROUP_RUNNER(subpersist) {
    RUN_TEST_CASE(subpersist, test_subpersist_init);
    RUN_TEST_CASE(subpersist, test_subpersist_load);
    RUN_TEST_CASE(subpersist, test_subpersist_save);
}

#if !defined(NO_MAIN)
static void runAllTests(void)
{
  RUN_TEST_GROUP(subpersist);
}

int main(int argc, const char *argv[])
{
  if (argc >= 2) {
    if (ofc_strcmp(argv[1], "--config") == 0) {
      ofc_strncpy(config_path, argv[2], OFC_MAX_PATH);
    }
  }

  return UnityMain(argc, argv, runAllTests);
}
#endif
