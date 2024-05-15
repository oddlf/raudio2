cmake_minimum_required(VERSION 3.17)

set(RAUDIO2_VERSION 1.0.2)

project(raudio2
    DESCRIPTION "raudio2: A simple and easy-to-use audio library based on raudio + miniaudio"
    HOMEPAGE_URL "https://github.com/oddlf/raudio2"
    VERSION ${RAUDIO2_VERSION}
    LANGUAGES C CXX
)

option(RAUDIO2_STANDALONE_PLUGIN "Make standalone plugin"      FALSE)
option(RAUDIO2_STATIC_CRT "Use static CRT library"             FALSE)
option(RAUDIO2_PACK_WITH_UPX "Pack programs with UPX"          FALSE)
option(RAUDIO2_BUILD_EXAMPLES "Build example programs"         TRUE)
option(RAUDIO2_INSTALL "Install library"                       TRUE)

option(RAUDIO2_ARCHIVE_GZIP "GZIP support"                     FALSE)
option(RAUDIO2_ARCHIVE_LIBARCHIVE "LibArchive support"         TRUE)

option(RAUDIO2_INPUT_DRFLAC "FLAC support (dr_flac)"           TRUE)
option(RAUDIO2_INPUT_FLAC "FLAC support (libflac)"             FALSE)
option(RAUDIO2_INPUT_GME "GME support"                         TRUE)
option(RAUDIO2_INPUT_DRMP3 "MP3 support (dr_mp3)"              FALSE)
option(RAUDIO2_INPUT_MPG123 "MP3 support (mpg123)"             TRUE)
option(RAUDIO2_INPUT_OPUS "OPUS support"                       TRUE)
option(RAUDIO2_INPUT_SNDFILE "SndFile support"                 FALSE)
option(RAUDIO2_INPUT_MODPLUG "Tracker music support (ModPlug)" TRUE)
option(RAUDIO2_INPUT_OPENMPT "Tracker music support (OpenMPT)" FALSE)
option(RAUDIO2_INPUT_STBVORBIS "Vorbis support (stb_vorbis)"   FALSE)
option(RAUDIO2_INPUT_VORBIS "Vorbis support (vorbisfile)"      TRUE)
option(RAUDIO2_INPUT_WAV "WAV support"                         TRUE)

set(RAUDIO2_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(RAUDIO2_ARCHIVE ${RAUDIO2_ROOT}/archive)
set(RAUDIO2_EXAMPLES ${RAUDIO2_ROOT}/examples)
set(RAUDIO2_EXTERNAL ${RAUDIO2_ROOT}/external)
set(RAUDIO2_INCLUDE ${RAUDIO2_ROOT}/include)
set(RAUDIO2_INPUT ${RAUDIO2_ROOT}/input)
set(RAUDIO2_SRC ${RAUDIO2_ROOT}/src)

find_path(RAUDIO2_MINIAUDIO_PATH "miniaudio.h" PATHS ${RAUDIO2_EXTERNAL})
message(STATUS "RAUDIO2_MINIAUDIO_PATH set to: ${RAUDIO2_MINIAUDIO_PATH}")

set(RAUDIO2_SOURCE_FILES
    ${RAUDIO2_SRC}/ArchivePluginIO.cpp
    ${RAUDIO2_SRC}/AudioBuffer.cpp
    ${RAUDIO2_SRC}/AudioDevice.cpp
    ${RAUDIO2_SRC}/AudioStream.cpp
    ${RAUDIO2_SRC}/FileIO.cpp
    ${RAUDIO2_SRC}/MemoryDataIO.cpp
    ${RAUDIO2_SRC}/MemoryIO.cpp
    ${RAUDIO2_SRC}/Music.cpp
    ${RAUDIO2_SRC}/raudio2.cpp
    ${RAUDIO2_SRC}/Utils.cpp
    ${RAUDIO2_SRC}/VirtualIO.cpp
)

if (RAUDIO2_STANDALONE_PLUGIN)
    add_subdirectory(${RAUDIO2_ARCHIVE}/gzip)
    add_subdirectory(${RAUDIO2_ARCHIVE}/libarchive)

    add_subdirectory(${RAUDIO2_INPUT}/drflac)
    add_subdirectory(${RAUDIO2_INPUT}/drmp3)
    add_subdirectory(${RAUDIO2_INPUT}/flac)
    add_subdirectory(${RAUDIO2_INPUT}/gme)
    add_subdirectory(${RAUDIO2_INPUT}/modplug)
    add_subdirectory(${RAUDIO2_INPUT}/mpg123)
    add_subdirectory(${RAUDIO2_INPUT}/openmpt)
    add_subdirectory(${RAUDIO2_INPUT}/opus)
    add_subdirectory(${RAUDIO2_INPUT}/sndfile)
    add_subdirectory(${RAUDIO2_INPUT}/stbvorbis)
    add_subdirectory(${RAUDIO2_INPUT}/vorbis)
    add_subdirectory(${RAUDIO2_INPUT}/wav)
endif()

add_library(${PROJECT_NAME} ${RAUDIO2_SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES PREFIX "")
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME}
    PUBLIC
        $<BUILD_INTERFACE:${RAUDIO2_INCLUDE}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
    PRIVATE
        ${RAUDIO2_MINIAUDIO_PATH} ${RAUDIO2_SRC}
)

if(RAUDIO2_ARCHIVE_GZIP)
    find_package(ZLIB QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE ZLIB::ZLIB)

    file(GLOB RAUDIO2_ARCHIVE_GZIP_SOURCES ${RAUDIO2_ARCHIVE}/gzip/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_ARCHIVE_GZIP_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_ARCHIVE}/gzip/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_ARCHIVE_GZIP)
endif()

if(RAUDIO2_ARCHIVE_LIBARCHIVE)
    find_package(LibArchive QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE LibArchive::LibArchive)

    file(GLOB RAUDIO2_ARCHIVE_LIBARCHIVE_SOURCES ${RAUDIO2_ARCHIVE}/libarchive/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_ARCHIVE_LIBARCHIVE_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_ARCHIVE}/libarchive/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_ARCHIVE_LIBARCHIVE)
endif()

if(RAUDIO2_INPUT_DRFLAC)
    find_path(RAUDIO2_DRFLAC_PATH "dr_flac.h" PATHS ${RAUDIO2_EXTERNAL})
    message(STATUS "RAUDIO2_DRFLAC_PATH set to: ${RAUDIO2_DRFLAC_PATH}")

    file(GLOB RAUDIO2_INPUT_DRFLAC_SOURCES ${RAUDIO2_INPUT}/drflac/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_DRFLAC_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/drflac/src ${RAUDIO2_DRFLAC_PATH})
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_DRFLAC)
endif()

if(RAUDIO2_INPUT_DRMP3)
    find_path(RAUDIO2_DRMP3_PATH "dr_mp3.h" PATHS ${RAUDIO2_EXTERNAL})
    message(STATUS "RAUDIO2_DRMP3_PATH set to: ${RAUDIO2_DRMP3_PATH}")

    file(GLOB RAUDIO2_INPUT_DRMP3_SOURCES ${RAUDIO2_INPUT}/drmp3/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_DRMP3_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/drmp3/src ${RAUDIO2_DRMP3_PATH})
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_DRMP3)
endif()

if(RAUDIO2_INPUT_FLAC)
    find_package(FLAC QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE FLAC::FLAC)

    file(GLOB RAUDIO2_INPUT_FLAC_SOURCES ${RAUDIO2_INPUT}/flac/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_FLAC_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/flac/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_FLAC)
endif()

if(RAUDIO2_INPUT_GME)
    list(APPEND CMAKE_MODULE_PATH "${RAUDIO2_INPUT}/gme/cmake")
    find_package(gme QUIET)
    find_package(ZLIB QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE gme::gme ZLIB::ZLIB)

    file(GLOB RAUDIO2_INPUT_GME_SOURCES ${RAUDIO2_INPUT}/gme/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_GME_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/gme/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_GME)
endif()

if(RAUDIO2_INPUT_MODPLUG)
    list(APPEND CMAKE_MODULE_PATH "${RAUDIO2_INPUT}/modplug/cmake")
    find_package(modplug QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE modplug::modplug)

    file(GLOB RAUDIO2_INPUT_MODPLUG_SOURCES ${RAUDIO2_INPUT}/modplug/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_MODPLUG_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/modplug/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_MODPLUG)
endif()

if(RAUDIO2_INPUT_MPG123)
    find_package(mpg123 CONFIG QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE MPG123::libmpg123)

    file(GLOB RAUDIO2_INPUT_MPG123_SOURCES ${RAUDIO2_INPUT}/mpg123/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_MPG123_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/mpg123/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_MPG123)
endif()

if(RAUDIO2_INPUT_OPENMPT)
    find_package(libopenmpt QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE libopenmpt::libopenmpt)

    file(GLOB RAUDIO2_INPUT_OPENMPT_SOURCES ${RAUDIO2_INPUT}/openmpt/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_OPENMPT_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/openmpt/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_OPENMPT)
endif()

if(RAUDIO2_INPUT_OPUS)
    find_package(OpusFile QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpusFile::opusfile)

    file(GLOB RAUDIO2_INPUT_OPUS_SOURCES ${RAUDIO2_INPUT}/opus/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_OPUS_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/opus/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_OPUS)
endif()

if(RAUDIO2_INPUT_SNDFILE)
    find_package(SndFile QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE SndFile::sndfile)

    file(GLOB RAUDIO2_INPUT_SNDFILE_SOURCES ${RAUDIO2_INPUT}/sndfile/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_SNDFILE_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/sndfile/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_SNDFILE)
endif()

if(RAUDIO2_INPUT_STBVORBIS)
    find_path(RAUDIO2_STBVORBIS_PATH "stb_vorbis.c" PATHS ${RAUDIO2_EXTERNAL})
    message(STATUS "RAUDIO2_STBVORBIS_PATH set to: ${RAUDIO2_STBVORBIS_PATH}")

    file(GLOB RAUDIO2_INPUT_STBVORBIS_SOURCES ${RAUDIO2_INPUT}/stbvorbis/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_STBVORBIS_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/stbvorbis/src ${RAUDIO2_STBVORBIS_PATH})
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_STBVORBIS)
endif()

if(RAUDIO2_INPUT_VORBIS)
    find_package(Vorbis QUIET)
    target_link_libraries(${PROJECT_NAME} PRIVATE Vorbis::vorbisfile)

    file(GLOB RAUDIO2_INPUT_VORBIS_SOURCES ${RAUDIO2_INPUT}/vorbis/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_VORBIS_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/vorbis/src)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_VORBIS)
endif()

if(RAUDIO2_INPUT_WAV)
    find_path(RAUDIO2_DRWAV_PATH "dr_wav.h" PATHS ${RAUDIO2_EXTERNAL})
    message(STATUS "RAUDIO2_DRWAV_PATH set to: ${RAUDIO2_DRWAV_PATH}")

    file(GLOB RAUDIO2_INPUT_WAV_SOURCES ${RAUDIO2_INPUT}/wav/src/*.cpp)
    target_sources(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT_WAV_SOURCES})
    target_include_directories(${PROJECT_NAME} PRIVATE ${RAUDIO2_INPUT}/wav/src ${RAUDIO2_DRWAV_PATH})
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_INPUT_WAV)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic -O3)
    if(WIN32)
        target_compile_options(${PROJECT_NAME} PRIVATE -ffunction-sections -fdata-sections)
    endif()
endif()

if(RAUDIO2_STATIC_CRT AND WIN32)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        set_property(TARGET ${PROJECT_NAME} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

if(BUILD_SHARED_LIBS)
    target_compile_definitions(${PROJECT_NAME} PRIVATE RAUDIO2_EXPORT_DLL)

    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        target_link_libraries(${PROJECT_NAME} PRIVATE -s)
        if(WIN32)
            target_link_libraries(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)
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
endif()

if(RAUDIO2_INSTALL)
    include(CMakePackageConfigHelpers)
    include(GNUInstallDirs)

    configure_package_config_file(
        cmake/${PROJECT_NAME}Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    install(
        TARGETS ${PROJECT_NAME}
        EXPORT ${PROJECT_NAME}Targets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        DIRECTORY ${RAUDIO2_INCLUDE}/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )

    install(
        EXPORT ${PROJECT_NAME}Targets
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        FILE ${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
    )

    install(
        FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
    )

    if(RAUDIO2_INPUT_GME)
        install(
            FILES "${RAUDIO2_INPUT}/gme/cmake/Findgme.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )
    endif()

    if(RAUDIO2_INPUT_MODPLUG)
        install(
            FILES "${RAUDIO2_INPUT}/modplug/cmake/Findmodplug.cmake"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}
        )
    endif()

    export(
        EXPORT ${PROJECT_NAME}Targets
        FILE ${CMAKE_CURRENT_BINARY_DIR}/cmake/${PROJECT_NAME}Targets.cmake
        NAMESPACE ${PROJECT_NAME}::
    )
endif()

if(RAUDIO2_BUILD_EXAMPLES)
    # Create a list of all examples
    set(examples)
    file(GLOB sources ${RAUDIO2_EXAMPLES}/*.c ${RAUDIO2_EXAMPLES}/*.cpp)
    list(APPEND examples ${sources})

    # Build each example
    foreach(example_source ${examples})
        # Create the basename for the example
        get_filename_component(example_name ${example_source} NAME)
        string(REPLACE ".cpp" "${OUTPUT_EXT}" example_name ${example_name})
        string(REPLACE ".c" "${OUTPUT_EXT}" example_name ${example_name})

        # Setup the example
        add_executable(${example_name} ${example_source})

        target_compile_features(${example_name} PRIVATE cxx_std_17)
        target_link_libraries(${example_name} PRIVATE ${PROJECT_NAME})

        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
            target_compile_options(${example_name} PRIVATE -Wall -Wpedantic -O3)
            if(WIN32)
                target_compile_options(${example_name} PRIVATE -ffunction-sections -fdata-sections)
            endif()
        endif()

        if(RAUDIO2_STATIC_CRT AND WIN32)
            if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
                set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static")
            elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
                set_property(TARGET ${example_name} PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
            endif()
        endif()
    endforeach()

    # Resources
    file(COPY ${RAUDIO2_EXAMPLES}/resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

macro(message_bool_option _NAME _VALUE)
    if(${_VALUE})
        message(STATUS "  ${_NAME}: enabled")
    else()
        message(STATUS "  ${_NAME}: disabled")
    endif()
endmacro()

message(STATUS "raudio2 will be built with the following options:")
message_bool_option("Build shared library" BUILD_SHARED_LIBS)
message_bool_option("Export function names" RAUDIO2_EXPORT_DLL)
message_bool_option("Use static CRT library" RAUDIO2_STATIC_CRT)
message_bool_option("Pack programs with UPX" RAUDIO2_PACK_WITH_UPX)
message_bool_option("Build example programs" RAUDIO2_BUILD_EXAMPLES)

message(STATUS "raudio2 will be built with the following archive plugins:")
message_bool_option("GZIP support" RAUDIO2_ARCHIVE_GZIP)
message_bool_option("LibArchive support" RAUDIO2_ARCHIVE_LIBARCHIVE)

message(STATUS "raudio2 will be built with the following input plugins:")
message_bool_option("FLAC support (dr_flac)" RAUDIO2_INPUT_DRFLAC)
message_bool_option("FLAC support (libflac)" RAUDIO2_INPUT_FLAC)
message_bool_option("GME support" RAUDIO2_INPUT_GME)
message_bool_option("MP3 support (dr_mp3)" RAUDIO2_INPUT_DRMP3)
message_bool_option("MP3 support (mpg123)" RAUDIO2_INPUT_MPG123)
message_bool_option("OPUS support" RAUDIO2_INPUT_OPUS)
message_bool_option("SndFile support" RAUDIO2_INPUT_SNDFILE)
message_bool_option("Tracker music support (ModPlug)" RAUDIO2_INPUT_MODPLUG)
message_bool_option("Tracker music support (OpenMPT)" RAUDIO2_INPUT_OPENMPT)
message_bool_option("Vorbis support (stb_vorbis)" RAUDIO2_INPUT_STBVORBIS)
message_bool_option("Vorbis support (vorbisfile)" RAUDIO2_INPUT_VORBIS)
message_bool_option("WAV support" RAUDIO2_INPUT_WAV)
