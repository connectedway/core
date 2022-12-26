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


if (OFC_FS_PIPE)
  list(APPEND TEST_EXTRA test_pipe.c)
endif()

if (OFC_FS_DARWIN)
   list(APPEND TEST_EXTRA test_fs_darwin.c)
endif()

if (OFC_FS_WIN32)
   list(APPEND TEST_EXTRA test_fs_windows.c)
endif()

if (OFC_FS_LINUX)
   list(APPEND TEST_EXTRA test_fs_linux.c)
endif()

if (OFC_FS_ANDROID)
   list(APPEND TEST_EXTRA test_fs_android.c)
endif()

if (OFC_PERSIST)
   list(APPEND TEST_EXTRA test_subpersist.c)
endif()

add_executable(test_all
        test_all.c
        test_timer.c
        test_event.c
	test_perf.c
        test_iovec.c
        test_waitq.c
        test_thread.c
        test_dg.c
        test_stream.c
        test_path.c
        test_file.c
	test_startup.c
	${TEST_EXTRA}
        )
target_compile_definitions(test_all PUBLIC NO_MAIN ON)
target_link_libraries(test_all PRIVATE of_core_static unityextras)
list(APPEND TEST_INSTALL test_all)

include_directories(
        ${of_core_SOURCE_DIR}/include
        ${unity_SOURCE_DIR}/src
        ${unity_SOURCE_DIR}/extras/fixture/src
        ${unity_SOURCE_DIR}/extras/memory/src
        ${CMAKE_CURRENT_SOURCE_DIR}
)

set(OPEN_FILES_HOME ${openfiles_SOURCE_DIR}/configs/darwin_debug.xml)

add_executable(test_timer test_timer.c test_startup.c)
target_link_libraries(test_timer PRIVATE of_core_static unityextras)
add_test(NAME timer COMMAND $<TARGET_FILE:test_timer> --config ${OPEN_FILES_HOME})
list(APPEND TEST_INSTALL test_timer)

add_executable(test_event test_event.c test_startup.c)
target_link_libraries(test_event PRIVATE of_core_static unityextras)
add_test(NAME event COMMAND $<TARGET_FILE:test_event> --config ${OPEN_FILES_HOME})
list(APPEND TEST_INSTALL test_event)

add_executable(test_perf test_perf.c test_startup.c)
target_link_libraries(test_perf PRIVATE of_core_static unityextras)
add_test(NAME perf COMMAND $<TARGET_FILE:test_perf>)
list(APPEND TEST_INSTALL test_perf)

add_executable(test_waitq test_waitq.c test_startup.c)
target_link_libraries(test_waitq PRIVATE of_core_static unityextras)
add_test(NAME waitq COMMAND $<TARGET_FILE:test_waitq> --config ${OPEN_FILES_HOME})
list(APPEND TEST_INSTALL test_waitq)

add_executable(test_thread test_thread.c test_startup.c)
target_link_libraries(test_thread PRIVATE of_core_static unityextras)
add_test(NAME thread COMMAND $<TARGET_FILE:test_thread> --config ${OPEN_FILES_HOME})
list(APPEND TEST_INSTALL test_thread)


add_executable(test_dg test_dg.c test_startup.c)
target_link_libraries(test_dg PRIVATE of_core_static unityextras)
add_test(NAME dg COMMAND $<TARGET_FILE:test_dg> --config ${OPEN_FILES_HOME})
list(APPEND TEST_INSTALL test_dg)

add_executable(test_stream test_stream.c test_startup.c)
target_link_libraries(test_stream PRIVATE of_core_static unityextras)
add_test(NAME stream COMMAND $<TARGET_FILE:test_stream> --config ${OPEN_FILES_HOME})
list(APPEND TEST_INSTALL test_stream)

add_executable(test_path test_path.c)
target_link_libraries(test_path PRIVATE of_core_static unityextras)
add_test(NAME path COMMAND $<TARGET_FILE:test_path>)
list(APPEND TEST_INSTALL test_path)

add_executable(test_iovec test_iovec.c)
target_link_libraries(test_iovec PRIVATE of_core_static unityextras)
add_test(NAME iovec COMMAND $<TARGET_FILE:test_iovec>)
list(APPEND TEST_INSTALL test_iovec)

if (OFC_FS_PIPE)
   add_executable(test_pipe test_pipe.c test_startup.c)
   target_link_libraries(test_pipe PRIVATE of_core_static unityextras)
   add_test(NAME pipe COMMAND $<TARGET_FILE:test_pipe> --config ${OPEN_FILES_HOME})
endif()
if (OFC_FS_DARWIN)
   add_executable(test_fs_darwin test_fs_darwin.c test_file.c)
   target_link_libraries(test_fs_darwin PRIVATE of_core_static unityextras)
   add_test(NAME fs_darwin COMMAND $<TARGET_FILE:test_fs_darwin>)
endif()
if (OFC_FS_WIN32)
   add_executable(test_fs_windows test_fs_windows.c test_file.c)
   target_link_libraries(test_fs_windows PRIVATE of_core_static unityextras)
   add_test(NAME fs_windows COMMAND $<TARGET_FILE:test_fs_windows>)
endif()
if (OFC_FS_LINUX)
   add_executable(test_fs_linux test_fs_linux.c test_file.c)
   target_link_libraries(test_fs_linux PRIVATE of_core_static unityextras)
   add_test(NAME fs_linux COMMAND $<TARGET_FILE:test_fs_linux>)
   list(APPEND TEST_INSTALL test_fs_linux)
endif()
if (OFC_FS_ANDROID)
   add_executable(test_fs_android test_fs_android.c test_file.c)
   target_link_libraries(test_fs_android PRIVATE of_core_static unityextras)
   add_test(NAME fs_android COMMAND $<TARGET_FILE:test_fs_android>)
endif()
if (OFC_PERSIST)
   add_executable(test_subpersist test_subpersist.c)
   target_link_libraries(test_subpersist PRIVATE of_core_static unityextras)
   add_test(NAME subpersist COMMAND $<TARGET_FILE:test_subpersist> --config ${OPEN_FILES_HOME})
endif()

install(TARGETS ${TEST_INSTALL}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/openfiles
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/openfiles
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/openfiles
        )
