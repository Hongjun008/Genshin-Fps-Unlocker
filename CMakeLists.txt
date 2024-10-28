cmake_minimum_required(VERSION 3.10)
project(Genshin_FPS_Unlocker VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

include_directories(headers)
AUX_SOURCE_DIRECTORY(src SRC)
FILE(GLOB UIC src/*.ui)
FILE(GLOB HEADERS headers/*.h)

set(PROJECT_SOURCES
    ${HEADERS}
    ${SRC}
    ${UIC}
    main.cpp
    resource.qrc
    info.rc
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(Genshin_FPS_Unlocker MANUAL_FINALIZATION ${PROJECT_SOURCES})
else()
    add_executable(Genshin_FPS_Unlocker ${PROJECT_SOURCES})
endif()

target_link_libraries(Genshin_FPS_Unlocker PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.Genshin_FPS_Unlocker)
endif()
set_target_properties(Genshin_FPS_Unlocker PROPERTIES
    LINK_FLAGS "/MANIFESTUAC:\"level='requireAdministrator' uiAccess='false'\" /SUBSYSTEM:WINDOWS"
    WIN32_EXECUTABLE TRUE
    ${BUNDLE_ID_OPTION}
)

include(GNUInstallDirs)
install(TARGETS Genshin_FPS_Unlocker
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(Genshin_FPS_Unlocker)
endif()