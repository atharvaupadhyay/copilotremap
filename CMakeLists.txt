cmake_minimum_required(VERSION 3.20)

project(CtrlPilot LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_definitions(UNICODE _UNICODE)

add_executable(CtrlPilot WIN32
    src/main.cpp
    src/hook.cpp
    src/hook.h
    src/state_machine.cpp
    src/state_machine.h
    src/input.cpp
    src/input.h
    src/config.h
    src/log.h
)

target_link_libraries(CtrlPilot PRIVATE user32)

if(MSVC)

    target_compile_options(CtrlPilot PRIVATE
        /W4
        /WX
        /permissive-
        /EHsc
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Release")

        set_property(TARGET CtrlPilot PROPERTY
            INTERPROCEDURAL_OPTIMIZATION TRUE)

        target_compile_options(CtrlPilot PRIVATE
            /O2
            /Gy
            /Gw
        )

        target_link_options(CtrlPilot PRIVATE
            /OPT:REF
            /OPT:ICF
        )

    endif()

else()

    target_compile_options(CtrlPilot PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Werror
    )

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        target_compile_definitions(CtrlPilot PRIVATE _DEBUG)
    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Debug")

        target_compile_options(CtrlPilot PRIVATE
            -O0
            -g3
        )

        target_compile_definitions(CtrlPilot PRIVATE
            _DEBUG
        )

    endif()

    if(CMAKE_BUILD_TYPE STREQUAL "Release")

        target_compile_options(CtrlPilot PRIVATE
            -O3
            -flto
        )

        target_link_options(CtrlPilot PRIVATE
            -flto
        )

    endif()

endif()