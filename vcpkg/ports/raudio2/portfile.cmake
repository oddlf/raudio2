vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO oddlf/raudio2
    REF "v${VERSION}"
    SHA512 54c20d0f7e4bbc82c1fe4280314071b1e3213eb98944709914ecc2162f78a70108e732f65f25495017e5301e339007c868b85076c99774d4ccdef673b3f4f832
)

vcpkg_check_features(OUT_FEATURE_OPTIONS FEATURE_OPTIONS
    FEATURES
        gzip       RAUDIO2_ARCHIVE_GZIP
        libarchive RAUDIO2_ARCHIVE_LIBARCHIVE
        drflac     RAUDIO2_INPUT_DRFLAC
        drmp3      RAUDIO2_INPUT_DRMP3
        flac       RAUDIO2_INPUT_FLAC
        gme        RAUDIO2_INPUT_GME
        modplug    RAUDIO2_INPUT_MODPLUG
        mpg123     RAUDIO2_INPUT_MPG123
        openmpt    RAUDIO2_INPUT_OPENMPT
        opus       RAUDIO2_INPUT_OPUS
        sndfile    RAUDIO2_INPUT_SNDFILE
        stbvorbis  RAUDIO2_INPUT_STBVORBIS
        vorbis     RAUDIO2_INPUT_VORBIS
        wav        RAUDIO2_INPUT_WAV
)

string(COMPARE EQUAL "${VCPKG_CRT_LINKAGE}" "static" RAUDIO2_STATIC_CRT)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DRAUDIO2_STANDALONE_PLUGIN=OFF
        -DRAUDIO2_STATIC_CRT=${RAUDIO2_STATIC_CRT}
        -DRAUDIO2_PACK_WITH_UPX=OFF
        -DRAUDIO2_BUILD_EXAMPLES=OFF
        -DRAUDIO2_INSTALL=ON
        ${FEATURE_OPTIONS}
)

vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(CONFIG_PATH lib/cmake/raudio2)
vcpkg_fixup_pkgconfig()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
file(INSTALL "${CMAKE_CURRENT_LIST_DIR}/usage" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}")
