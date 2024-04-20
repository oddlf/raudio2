cmake_minimum_required(VERSION 3.17)

project(raudio2-wav
    DESCRIPTION "raudio2 wav input plugin"
    LANGUAGES C CXX
)

option(RAUDIO2_STANDALONE_PLUGIN "Make standalone plugin" TRUE)
option(RAUDIO2_STATIC_CRT "Use static CRT library" TRUE)
option(RAUDIO2_PACK_WITH_UPX "Pack programs with UPX" FALSE)

set(RAUDIO2_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(RAUDIO2_EXTERNAL ${RAUDIO2_ROOT}/../../external)
set(RAUDIO2_INCLUDE ${RAUDIO2_ROOT}/../../include)
set(RAUDIO2_SRC ${RAUDIO2_ROOT}/src)

find_path(RAUDIO2_DRWAV_PATH "dr_wav.h" PATHS ${RAUDIO2_EXTERNAL})
message(STATUS "RAUDIO2_DRWAV_PATH set to: ${RAUDIO2_DRWAV_PATH}")

set(RAUDIO2_SOURCE_FILES
    ${RAUDIO2_SRC}/raudio2_wav.cpp
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
    PRIVATE ${RAUDIO2_DRWAV_PATH} ${RAUDIO2_INCLUDE}
)

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
