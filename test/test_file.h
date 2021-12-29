
#include "ofc/types.h"

/*
 * Startup Entry point to the test
 * 
 * \param hScheduler
 * The scheduler that this test should create apps within (if any)
 * In our case, the file system test does not create any apps.
 */
#if !defined(__TEST_FILE_H__)
#define __TEST_FILE_H__

#if defined(__cplusplus)
extern "C"
{
#endif

OFC_INT test_file(OFC_LPCSTR test_root);

#if defined(__cplusplus)
}
#endif

#endif
