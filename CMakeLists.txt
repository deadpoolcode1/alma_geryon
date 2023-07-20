# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
set(BOARD_ROOT ${CMAKE_CURRENT_LIST_DIR})
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

if(DEFINED ENV{VERSION})
project(app LANGUAGES C VERSION $ENV{VERSION})
else()
project(app LANGUAGES C VERSION 0.1.0)
endif()

set(PROJECT_VERSION_RC 7)

if(PROJECT_VERSION_RC)
  set(PROJECT_VERSION_SUFFIX "rc${PROJECT_VERSION_RC}")
else()
  set(PROJECT_VERSION_SUFFIX "")
endif()

if(PROJECT_VERSION_SUFFIX)
  set(PROJECT_VERSION ${PROJECT_VERSION}-${PROJECT_VERSION_SUFFIX})
endif()


configure_file(app_version.h.in ${CMAKE_BINARY_DIR}/app/include/app_version.h)

target_sources(app PRIVATE 
  src/main.c
  src/pwm.c  
  src/protocol.c
  src/io.c
  src/app_event_manager.c
  src/cset.c
  src/cmon.c
  src/mcp466.c
)
