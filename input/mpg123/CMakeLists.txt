cmake_minimum_required(VERSION 3.17)

project(raudio2-mpg123
    DESCRIPTION "raudio2 mpg123 input plugin"
    LANGUAGES C CXX
)

option(RAUDIO2_STANDALONE_PLUGIN "Make standalone plugin" TRUE)
option(RAUDIO2_STATIC_CRT "Use static CRT library" TRUE)
option(RAUDIO2_PACK_WITH_UPX "Pack programs with UPX" FALSE)

find_package(mpg123 CONFIG QUIET)

if(NOT ${mpg123_FOUND})
    SET(RAUDIO2_INPUT_MPG123 FALSE CACHE BOOL "MP3 support (mpg123)" FORCE)
    SET(RAUDIO2_INPUT_DRMP3 TRUE CACHE BOOL "MP3 support (dr_mp3)" FORCE)
    MESSAGE(STATUS "Dependencies not found. Disabling mpg123 support support and enabling dr_mp3")
    return()
endif()

set(RAUDIO2_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(RAUDIO2_INCLUDE ${RAUDIO2_ROOT}/../../include)
set(RAUDIO2_SRC ${RAUDIO2_ROOT}/src)

set(RAUDIO2_SOURCE_FILES
    ${RAUDIO2_SRC}/raudio2_mpg123.cpp
)

add_library(${PROJECT_NAME} ${RAUDIO2_SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
if(RAUDIO2_STANDALONE_PLUGIN)
    target_compile_definitions(${PROJECT_NAME} PUBLIC RAUDIO2_EXPORT_DLL RAUDIO2_STANDALONE_PLUGIN)
endif()
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${RAUDIO2_SRC}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE ${RAUDIO2_INCLUDE}
)
target_link_libraries(${PROJECT_NAME} PRIVATE MPG123::libmpg123)

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic -O3)
    if(BUILD_SHARED_LIBS)
        target_link_libraries(${PROJECT_NAME} PRIVATE -s)
    endif()
    if(WIN32)
        target_compile_options(${PROJECT_NAME} PRIVATE -ffunction-sections -fdata-sections)
        if(BUILD_SHARED_LIBS)
            target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)
        endif()
    endif()
endif()

if(RAUDIO2_STATIC_CRT AND WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(RAUDIO2_PACK_WITH_UPX)
    include(FindSelfPackers)
    if(SELF_PACKER_FOR_EXECUTABLE)
        ADD_CUSTOM_COMMAND(
            COMMAND ${SELF_PACKER_FOR_EXECUTABLE} $<TARGET_FILE:${PROJECT_NAME}>
            ARGS ${SELF_PACKER_FOR_EXECUTABLE_FLAGS} -9q
            TARGET ${PROJECT_NAME}
        )
    endif()
endif()
