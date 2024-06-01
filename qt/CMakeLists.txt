# technically, we need 3.21 for Qt6 support, but, otherwise we can get away with a much older version...
cmake_minimum_required(VERSION 3.2)
project(stash)

enable_language(CXX)

set(STASH_QT_VERSION 5)
#set(STASH_QT_VERSION 6)

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	# We don't really care about using the Qt port on MacOS, but it does build, and is an additional
	# testing point, so...
	set(STASH_QT_VERSION 5)
endif()

if(${STASH_QT_VERSION} LESS 6)
    # Qt 4 and 5
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED on)
    # so that QtCreator uses modern C++...
    set(QMAKE_CXXFLAGS "-std=c++11")
    set(EXTRA_FLAGS "-std=c++11")
else()
    # Qt 6
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    # so that QtCreator uses modern C++ - note, this needs to be c++11, it doesn't affect the compilation itself...
    set(QMAKE_CXXFLAGS "-std=c++11")
    set(EXTRA_FLAGS "-std=c++17")
endif()

add_definitions(-DSTASH_QT_VER=${STASH_QT_VERSION})

#set(DEBUG_WARNINGS "-Wall -Wextra")
#set(DEBUG_WARNINGS "${DEBUG_WARNINGS} -Weverything")
set(DEBUG_WARNINGS_SUPPRESS "-Wno-unused-function -Wno-unused-parameter -Wno-unused-variable")
#set(MAIN_DEBUG_CPP_FLAGS "-g -D_GLIBCXX_DEBUG ${DEBUG_WARNINGS} ${DEBUG_WARNINGS_SUPPRESS}")
set(MAIN_DEBUG_CPP_FLAGS "-g ${DEBUG_WARNINGS} ${DEBUG_WARNINGS_SUPPRESS}")

#set(MAIN_RELEASE_CPP_FLAGS "-O2 -g -fPIC")
set(MAIN_RELEASE_CPP_FLAGS "-O2 -fPIC")

set(CMAKE_CXX_FLAGS_DEBUG "${EXTRA_FLAGS} ${MAIN_DEBUG_CPP_FLAGS}")
set(CMAKE_CXX_FLAGS_RELEASE "${EXTRA_FLAGS} ${MAIN_RELEASE_CPP_FLAGS}")

if(${STASH_QT_VERSION} EQUAL 4)
	# Qt 4
	find_package(Qt4 REQUIRED)
elseif(${STASH_QT_VERSION} EQUAL 5)
    # Qt 5
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	set(CMAKE_AUTOMOC ON)
    # MacOS seems to sort of need this, but as we don't *really*
    # care about MacOS for the Qt port, it's disabled for the moment.
	#set(QT_USE_QTOPENGL TRUE)
	find_package(Qt5Widgets)
else()
	# Qt 6
	find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
	set(CMAKE_INCLUDE_CURRENT_DIR ON)
	set(CMAKE_AUTOMOC ON)
endif()

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

if(${STASH_QT_VERSION} EQUAL 4)
	include(${QT_USE_FILE})
	add_definitions(${QT_DEFINITIONS})
	QT4_WRAP_CPP(stash_HEADERS_MOC ${stash_qt_HEADERS})
	QT4_ADD_RESOURCES(stash_RESOURCES_RCC ${stash_qt_RESOURCES})
elseif(${STASH_QT_VERSION} EQUAL 5)
	QT5_WRAP_CPP(stash_HEADERS_MOC ${stash_qt_HEADERS})
	QT5_ADD_RESOURCES(stash_RESOURCES_RCC ${stash_qt_RESOURCES})
else()
	QT6_WRAP_CPP(stash_HEADERS_MOC ${stash_qt_HEADERS})
	QT6_ADD_RESOURCES(stash_RESOURCES_RCC ${stash_qt_RESOURCES})
endif()

add_executable(stash ${stash_core_SOURCES} ${stash_qt_SOURCES} ${stash_HEADERS_MOC} ${stash_RESOURCES_RCC})

# OS X needs iconv at link time, but Linux doesn't for some reason.
if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    if(${STASH_QT_VERSION} EQUAL 4)
    	target_link_libraries(stash "iconv" ${QT_LIBRARIES} ${OPENGL_LIBRARY})
    elseif(${STASH_QT_VERSION} EQUAL 5)
        target_link_libraries(stash "iconv" Qt5::Widgets ${OPENGL_LIBRARY})
    else()
        target_link_libraries(stash "iconv" Qt6::Widgets ${OPENGL_LIBRARY})
    endif()
else()
    if(${STASH_QT_VERSION} EQUAL 4)
        target_link_libraries(stash ${QT_LIBRARIES} ${OPENGL_LIBRARY})
    elseif(${STASH_QT_VERSION} EQUAL 5)
        target_link_libraries(stash Qt5::Widgets ${OPENGL_LIBRARY})
    else()
        target_link_libraries(stash Qt6::Widgets ${OPENGL_LIBRARY})
    endif()
endif()


