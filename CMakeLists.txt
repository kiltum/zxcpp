cmake_minimum_required(VERSION 3.19)
project(zxcpp LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets LinguistTools)

qt_standard_project_setup()

qt_add_executable(zxcpp
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    settingswindow.h settingswindow.cpp settingswindow.ui
    src/z80_cb_opcodes.cpp
    src/z80_dd_opcodes.cpp
    src/z80_ddcb_opcodes.cpp
    src/z80_ed_opcodes.cpp
    src/z80_fd_opcodes.cpp
    src/z80_fdcb_opcodes.cpp
    src/z80_opcodes.cpp
    src/z80.cpp
    src/ay8912.cpp
    src/kempston.cpp
    src/memory.cpp
    src/port.cpp
    src/sound.cpp
    src/tape.cpp
    src/ula.cpp
    include/emu.h src/emu.cpp
    lib/vgm_decoder/src/chips/ay-3-8910.cpp
)

qt_add_translations(
    TARGETS zxcpp
    TS_FILES zxcpp_ru_RU.ts
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/lib/vgm_decoder/include
)

find_package(libzip CONFIG REQUIRED)
find_package(SDL3 REQUIRED CONFIG REQUIRED COMPONENTS SDL3-shared)

target_link_libraries(zxcpp
    PRIVATE
        Qt::Core
        Qt::Widgets
        libzip::zip
        SDL3::SDL3
)

include(GNUInstallDirs)

install(TARGETS zxcpp
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET zxcpp
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
