include(FetchContent)
FetchContent_Declare(unity
  URL ${openfiles_SOURCE_DIR}/Unity
  )
FetchContent_MakeAvailable(unity)

add_library(unityextras INTERFACE)
target_link_libraries(unityextras INTERFACE unity)
target_include_directories(unityextras INTERFACE
    ${unity_SOURCE_DIR}/extras/fixture/src
    ${unity_SOURCE_DIR}/extras/memory/src
    )

target_sources(unityextras INTERFACE
    ${unity_SOURCE_DIR}/extras/fixture/src/unity_fixture.c
    ${unity_SOURCE_DIR}/extras/memory/src/unity_memory.c
    )


#add_executable(test_all
#    test_all.c
#    test_timer.c
#    test_event.c
#    test_waitq.c
#    test_thread.c
#    test_dgram.c
#    test_stream.c
#    test_path.c
#    )
#target_compile_definitions(test_all PUBLIC NO_MAIN ON)
#target_link_libraries(test_all INTERFACE of_core_shared)

include_directories(
	${of_core_BINARY_DIR}
	${of_core_SOURCE_DIR}/include
	${unity_SOURCE_DIR}/src
	${unity_SOURCE_DIR}/extras/fixture/src
	${unity_SOURCE_DIR}/extras/memory/src
	)


add_executable(test_timer test_timer.c)
target_link_libraries(test_timer PRIVATE of_core_shared unityextras)
add_test(timer test_timer COMMAND $<TARGET_FILE:test_timer>)

add_executable(test_event test_event.c)
target_link_libraries(test_event PRIVATE of_core_shared unityextras)
add_test(event test_event COMMAND $<TARGET_FILE:test_event>)

add_executable(test_waitq test_waitq.c)
target_link_libraries(test_waitq PRIVATE of_core_shared unityextras)
add_test(waitq test_waitq COMMAND $<TARGET_FILE:test_waitq>)

add_executable(test_thread test_thread.c)
target_link_libraries(test_thread PRIVATE of_core_shared unityextras)
add_test(thread test_thread COMMAND $<TARGET_FILE:test_thread>)

add_executable(test_dg test_dg.c)
target_link_libraries(test_dg PRIVATE of_core_shared unityextras)
add_test(dg test_dg COMMAND $<TARGET_FILE:test_dg>)

add_executable(test_stream test_stream.c)
target_link_libraries(test_stream PRIVATE of_core_shared unityextras)
add_test(timer test_stream COMMAND $<TARGET_FILE:test_stream>)

#add_executable(test_path test_path.c)
#target_link_libraries(test_path INTERFACE of_core_shared unityextras)
#add_test(timer test_path COMMAND $<TARGET_FILE:test_path>)
