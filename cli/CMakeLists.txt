CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(stash_cli)

ENABLE_LANGUAGE(CXX)

SET(CMAKE_CXX_STANDARD 11)
SET(CMAKE_CXX_STANDARD_REQUIRED on)

SET(QMAKE_CXXFLAGS "-std=c++11")

SET(CMAKE_CXX_FLAGS_RELEASE "-O2 -std=c++11")
SET(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++11")

IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Debug CACHE STRING
	  "Choose the type of build, options are: None Debug Release"
	  FORCE)
ENDIF(NOT CMAKE_BUILD_TYPE)
IF(CMAKE_FIRST_RUN)
	message(STATUS "Setting Build Type to: ${CMAKE_BUILD_TYPE}")
ENDIF()

# Add core stuff from directory up...
#include_directories("../core")
FILE(GLOB_RECURSE stash_core_SOURCES "${CMAKE_SOURCE_DIR}/../core/*.cpp")

INCLUDE_DIRECTORIES("src")
FILE(GLOB_RECURSE stash_cli_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

ADD_EXECUTABLE(stash_cli ${stash_core_SOURCES} ${stash_cli_SOURCES})
TARGET_LINK_LIBRARIES(stash_cli "iconv")
