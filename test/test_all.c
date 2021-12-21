#include "unity_fixture.h"

static void runAllTests(void) {
	RUN_TEST_GROUP(timer)
	RUN_TEST_GROUP(event)
        RUN_TEST_GROUP(waitq)
        RUN_TEST_GROUP(thread)
        RUN_TEST_GROUP(dgram)
        RUN_TEST_GROUP(stream)
        RUN_TEST_GROUP(path)
}

int main(int argc, const char *argv[]) {
	return UnityMain(argc, argv, runAllTests);
}
