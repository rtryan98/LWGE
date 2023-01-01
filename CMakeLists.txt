CMAKE_MINIMUM_REQUIRED(VERSION 3.18 FATAL_ERROR)

INCLUDE("scripts/LWGE.cmake")

PROJECT(LWGE)

SET(LWGE_NAME "LWGE")
FILE(GLOB_RECURSE LWGE_FILES "LWGE/src/*.cpp" "LWGE/include/*.hpp")
GROUP_FILE_TREE("${LWGE_FILES}")
SET(LWGE_DEMO_NAME "LWGE-Demo")
FILE(GLOB_RECURSE LWGE_DEMO_FILES "LWGE-Demo/src/*.cpp" "LWGE-Demo/include/*.hpp")
GROUP_FILE_TREE("${LWGE_DEMO_FILES}")

SET(CMAKE_CXX_STANDARD_REQUIRED ON)
SET(CMAKE_CONFIGURATION_TYPES "Debug;RelWithDebInfo;Release" CACHE STRING "" FORCE)

ADD_LIBRARY(${LWGE_NAME} ${LWGE_FILES})
TARGET_INCLUDE_DIRECTORIES(${LWGE_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/LWGE/include/)
SET_TARGET_PROPERTIES(${LWGE_NAME} PROPERTIES CXX_STANDARD 20)
ADD_EXECUTABLE(${LWGE_DEMO_NAME} ${LWGE_DEMO_FILES})
TARGET_INCLUDE_DIRECTORIES(${LWGE_DEMO_NAME} PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/LWGE-Demo/include/)
SET_TARGET_PROPERTIES(${LWGE_DEMO_NAME} PROPERTIES CXX_STANDARD 20)

IF(WIN32)
    MESSAGE(STATUS "Building on and for Win32.")
    TARGET_COMPILE_DEFINITIONS(${LWGE_NAME} PUBLIC
        "LWGE_TARGET_WINDOWS=1")
ELSE()
    MESSAGE(FATAL_ERROR "Incompatible target platform. Aborting")
ENDIF()

TARGET_LINK_LIBRARIES(${LWGE_DEMO_NAME}
    ${LWGE_NAME})

OPTION(LWGE_GRAPHICS_DEBUG "Enable validation layers and debug logging." ON)

if(LWGE_GRAPHICS_DEBUG)
    TARGET_COMPILE_DEFINITIONS(${LWGE_NAME} PRIVATE
        "LWGE_GRAPHICS_DEBUG=1")
endif()

IF(CMAKE_GENERATOR MATCHES "Visual Studio")
    MESSAGE(VERBOSE "Using Visual Studio.")
    SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY
        VS_STARTUP_PROJECT ${LWGE_DEMO_NAME})
    SET_TARGET_PROPERTIES(${LWGE_DEMO_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")
ENDIF()
IF(MSVC)
    TARGET_COMPILE_OPTIONS(${LWGE_NAME} PRIVATE
        "/wd26812" # The enum type 'type-name' is unscoped. Prefer 'enum class' over 'enum'
        "/WX" "/W4" "/MP"
        )
ENDIF()