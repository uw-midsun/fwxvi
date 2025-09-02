# CMakeLists.txt (project root: gui/)
cmake_minimum_required(VERSION 3.16)
project(mpxe_gui LANGUAGES CXX)

# ---- Qt convenience ----
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# ---- Find Qt (prefer Qt6; fall back to Qt5) ----
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)

# ---- Collect sources ----
# Explicit globs per folder so the tree stays clean and predictable
file(GLOB_RECURSE MODELS_SRC CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/models/src/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/models/src/*.cpp
)
file(GLOB_RECURSE UI_SRC CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/src/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/src/*.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/src/*.cxx
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/src/*.ui
)
file(GLOB_RECURSE UTILS_SRC CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/src/*.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/src/*.cpp
)
file(GLOB_RECURSE QRC_FILES CONFIGURE_DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/assets/*.qrc
)

set(APP_MAIN ${CMAKE_CURRENT_SOURCE_DIR}/main.cc)

set(APP_SOURCES
  ${APP_MAIN}
  ${MODELS_SRC}
  ${UI_SRC}
  ${UTILS_SRC}
  ${QRC_FILES}
)

# (Optional) add headers to the target so IDEs show them and automoc sees Q_OBJECT
file(GLOB_RECURSE MODELS_HDR CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/models/inc/*.h)
file(GLOB_RECURSE UI_HDR     CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/ui/inc/*.h)
file(GLOB_RECURSE UTILS_HDR  CONFIGURE_DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/utils/inc/*.h)

add_executable(mpxe_gui
  ${APP_SOURCES}
  ${MODELS_HDR}
  ${UI_HDR}
  ${UTILS_HDR}
)

# ---- Includes ----
target_include_directories(mpxe_gui PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/models/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/ui/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/utils/inc
)

# ---- Link Qt ----
target_link_libraries(mpxe_gui PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Network
)

# ---- Platform niceties ----
set_target_properties(mpxe_gui PROPERTIES
  WIN32_EXECUTABLE TRUE       # no console on Windows
  MACOSX_BUNDLE TRUE
)

# MSVC: ensure UTF-8 sources
if(MSVC)
  target_compile_options(mpxe_gui PRIVATE /utf-8)
endif()

# ---- (Optional) Nice IDE grouping ----
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
  ${APP_SOURCES} ${MODELS_HDR} ${UI_HDR} ${UTILS_HDR}
)
