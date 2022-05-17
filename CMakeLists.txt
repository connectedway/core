cmake_minimum_required(VERSION 3.20.0)
project(of_core VERSION 1.0.1 DESCRIPTION "OpenFiles Core Library")

set(SRCS
        src/app.c
        src/console.c
        src/core.c
	src/dce.c
        src/dom.c
        src/env.c
        src/event.c
        src/file.c
	src/file_call.c
        src/framework.c
        src/fs.c
        src/handle.c
        src/heap.c
        src/libc.c
        src/lock.c
        src/message.c
        src/net.c
        src/ntop.c
        src/path.c
        src/persist.c
        src/process.c
        src/queue.c
        src/sax.c
        src/sched.c
        src/socket.c
        src/thread.c
        src/time.c
        src/timer.c
        src/waitq.c
        src/waitset.c
        )

if (DEFINED CACHE{OFC_FS_PIPE})
    message("Adding FS Pipe")
    list(APPEND fs_library_list of_core_fs_pipe)
endif ()

if (DEFINED CACHE{OFC_FS_DARWIN})
    message("Adding FS Darwin")
    list(APPEND fs_library_list of_core_fs_darwin)
endif ()

if (DEFINED CACHE{OFC_FS_WIN32})
    message("Adding FS Windows")
    list(APPEND fs_library_list of_core_fs_windows)
endif ()

if (DEFINED CACHE{OFC_FS_LINUX})
    message("Adding FS Linux")
    list(APPEND fs_library_list of_core_fs_linux)
endif ()

if (DEFINED CACHE{OFC_FS_ANDROID})
    message("Adding FS Android")
    list(APPEND fs_library_list of_core_fs_android)
endif ()

if (DEFINED CACHE{OFC_FS_BOOKMARKS})
    message("Adding FS Bookmarks")
    list(APPEND fs_library_list of_core_fs_bookmarks)
endif ()

add_library(of_core OBJECT ${SRCS})
set_property(TARGET of_core PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(of_core_static STATIC $<TARGET_OBJECTS:of_core>)
add_library(of_core_shared SHARED $<TARGET_OBJECTS:of_core>)

set_target_properties(of_core_shared PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(of_core_static PROPERTIES VERSION ${PROJECT_VERSION})

set_target_properties(of_core_shared PROPERTIES SOVERSION 1)
set_target_properties(of_core_static PROPERTIES SOVERSION 1)

target_include_directories(of_core PUBLIC include)

target_link_libraries(of_core_static PUBLIC
        ${fs_library_list}
        of_core_${OFC_HEAP_IMPL}
        of_core_${OFC_IMPL}
        )
target_link_libraries(of_core_shared PUBLIC
        ${fs_library_list}
        of_core_${OFC_HEAP_IMPL}
        of_core_${OFC_IMPL}
        )

add_subdirectory(test)
