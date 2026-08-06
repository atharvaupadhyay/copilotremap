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

if(MSVC)
    # Rigorous warnings and standards compliance
    target_compile_options(CtrlPilot PRIVATE /W4 /WX /permissive- /EHsc)
    
    # Release optimizations
    if (CMAKE_BUILD_TYPE STREQUAL "Release" OR NOT CMAKE_BUILD_TYPE)
        set_property(TARGET CtrlPilot PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE) # /GL and /LTCG
        target_compile_options(CtrlPilot PRIVATE /O2 /Gy /Gw)
        target_link_options(CtrlPilot PRIVATE /OPT:REF /OPT:ICF)
    endif()
endif()

# Link User32 for hooks and inputs
target_link_libraries(CtrlPilot PRIVATE user32)
