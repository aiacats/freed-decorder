include(FetchContent)

FetchContent_Declare(
    oscpp
    GIT_REPOSITORY https://github.com/kaoskorobase/oscpp.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(oscpp)

if(NOT TARGET oscpp)
    add_library(oscpp INTERFACE)
    target_include_directories(oscpp INTERFACE
        ${oscpp_SOURCE_DIR}/include
    )
endif()
