find_path(modplug_INCLUDE_DIR NAMES modplug.h libmodplug/modplug.h)
mark_as_advanced(modplug_INCLUDE_DIR)

find_library(modplug_LIBRARY NAMES modplug)
mark_as_advanced(modplug_LIBRARY)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(modplug REQUIRED_VARS
    modplug_INCLUDE_DIR
    modplug_LIBRARY
)

if(modplug_FOUND)
    set(modplug_INCLUDE_DIRS ${modplug_INCLUDE_DIR})
    set(modplug_LIBRARIES ${modplug_LIBRARY})
    if(NOT TARGET modplug::modplug)
        add_library(modplug::modplug UNKNOWN IMPORTED)
        set_target_properties(modplug::modplug PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
            IMPORTED_LOCATION "${modplug_LIBRARY}"
            INTERFACE_INCLUDE_DIRECTORIES "${modplug_INCLUDE_DIR}")
    endif()
endif()
