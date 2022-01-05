cmake_minimum_required(VERSION 3.20.0)
project(of_core VERSION 1.0.1 DESCRIPTION "OpenFiles Core Library")

set(SRCS
        src/app.c
        src/console.c
        src/core.c
        src/dom.c
        src/env.c
        src/event.c
        src/file.c
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

if (DEFINED CACHE{OFC_FS_BOOKMARKS})
    message("Adding FS Bookmarks")
    list(APPEND fs_library_list of_core_fs_bookmarks)
endif ()

add_library(of_core_static STATIC ${SRCS})
# add_library(of_core_shared SHARED ${SRCS})

# set_target_properties(of_core_shared PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(of_core_static PROPERTIES VERSION ${PROJECT_VERSION})
# set_target_properties(of_core_shared PROPERTIES SOVERSION 1)
set_target_properties(of_core_static PROPERTIES SOVERSION 1)

target_include_directories(of_core_static PUBLIC include)
# target_include_directories(of_core_shared PUBLIC include)
target_link_libraries(of_core_static PRIVATE
        of_core_${OFC_HEAP_IMPL}
        of_core_${OFC_IMPL}
        ${fs_library_list}
        )
#target_link_libraries(of_core_shared PRIVATE
#        of_core_${OFC_HEAP_IMPL}
#        of_core_${OFC_IMPL}
#        ${fs_library_list}
#        )

enable_testing()
add_subdirectory(test)
