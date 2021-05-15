cmake_minimum_required(VERSION 3.0)
project(stash)

enable_language(CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED on)

# so that QtCreator uses modern C++..
set(QMAKE_CXXFLAGS "-std=c++11")

#set(DEBUG_WARNINGS "-Wall -Wextra")
#set(DEBUG_WARNINGS "${DEBUG_WARNINGS} -Weverything")
#set(DEBUG_WARNINGS_SUPPRESS "-Wno-unused-function -Wno-unused-parameter -Wno-unused-variable")
#set(MAIN_DEBUG_CPP_FLAGS "-g -D_GLIBCXX_DEBUG ${DEBUG_WARNINGS} ${DEBUG_WARNINGS_SUPPRESS}")
set(MAIN_DEBUG_CPP_FLAGS "-g ${DEBUG_WARNINGS} ${DEBUG_WARNINGS_SUPPRESS}")

#set(MAIN_RELEASE_CPP_FLAGS "-O2 -g -fPIC -ffast-math")
set(MAIN_RELEASE_CPP_FLAGS "-O2 -fPIC -ffast-math")

set(EXTRA_FLAGS "-std=c++11")

set(CMAKE_CXX_FLAGS_DEBUG "${EXTRA_FLAGS} ${MAIN_DEBUG_CPP_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${EXTRA_FLAGS} ${MAIN_RELEASE_CPP_FLAGS}")

# To build with Qt 4.x, set this to 0
set(QT5 1)
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    # We don't really care about using the Qt port on MacOS, but it does build, and is an additional
    # testing point, so...
	set(QT5 1)
endif()

if(${QT5} MATCHES 0)
	# Qt 4
	find_package(Qt4 REQUIRED)
else()
	# Qt 5
	add_definitions(-DSTASH_QT_5)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	set(CMAKE_AUTOMOC ON)
	find_package(Qt5Widgets)
endif()
#set(QT_USE_QTOPENGL TRUE)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING
	  "Choose the type of build, options are: None Debug Release"
	  FORCE)
endif(NOT CMAKE_BUILD_TYPE)
if(CMAKE_FIRST_RUN)
	message(STATUS "Setting Build Type to: ${CMAKE_BUILD_TYPE}")
endif()

# Add core stuff from directory up...
#include_directories("../core")
file(GLOB_RECURSE stash_core_SOURCES "${CMAKE_SOURCE_DIR}/../core/*.cpp")

include_directories("src")
file(GLOB_RECURSE stash_qt_SOURCES "${CMAKE_SOURCE_DIR}/src/*.cpp")

file(GLOB_RECURSE stash_qt_qobject_headers "${CMAKE_SOURCE_DIR}/src/*.h")

set(stash_qt_HEADERS ${stash_qt_qobject_headers})
set(stash_qt_RESOURCES "${CMAKE_SOURCE_DIR}/stash.qrc")

if(${QT5} MATCHES 0)
	include(${QT_USE_FILE})
	add_definitions(${QT_DEFINITIONS})
	QT4_WRAP_CPP(stash_HEADERS_MOC ${stash_qt_HEADERS})
	QT4_ADD_RESOURCES(stash_RESOURCES_RCC ${stash_qt_RESOURCES})
else()
	QT5_WRAP_CPP(stash_HEADERS_MOC ${stash_qt_HEADERS})
	QT5_ADD_RESOURCES(stash_RESOURCES_RCC ${stash_qt_RESOURCES})
endif()

add_executable(stash ${stash_core_SOURCES} ${stash_qt_SOURCES} ${stash_HEADERS_MOC} ${stash_RESOURCES_RCC})

# OS X needs iconv at link time, but Linux doesn't for some reason.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if(${QT5} MATCHES 0)
    	target_link_libraries(stash "iconv" ${QT_LIBRARIES} ${OPENGL_LIBRARY})
    else()
        target_link_libraries(stash "iconv" Qt5::Widgets ${OPENGL_LIBRARY})
    endif()
else()
    if(${QT5} MATCHES 0)
        target_link_libraries(stash ${QT_LIBRARIES} ${OPENGL_LIBRARY})
    else()
        target_link_libraries(stash Qt5::Widgets ${OPENGL_LIBRARY})
    endif()
endif()


