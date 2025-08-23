# CMakeLists.txt (project root)
cmake_minimum_required(VERSION 3.16)
project(mpxe_gui LANGUAGES CXX)

# ---- Qt convenience ----
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)    # auto-handle .ui (if any)
set(CMAKE_AUTORCC ON)    # auto-handle .qrc (if any)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# (Optional) Better defaults on Qt6 6.3+
# if(QT_VERSION_MAJOR EQUAL 6)
#   include(${CMAKE_CURRENT_LIST_DIR}/cmake/Qt6Policies.cmake)
#   qt_standard_project_setup()       # if available; harmless to omit
# endif()

# ---- Find Qt (try Qt6, fall back to Qt5) ----
# Add Core explicitly to COMPONENTS; Widgets pulls Gui/Core anyway,
# but listing Core makes find_package more predictable across platforms.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets Network)

# ---- Sources / headers ----
# GLOB is fine for now; for production consider listing sources explicitly.
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
  src/*.cpp src/*.cc src/*.cxx
  src/*.ui
  res/*.qrc
)
file(GLOB_RECURSE APP_HEADERS CONFIGURE_DEPENDS
  inc/*.h inc/*.hpp
)

add_executable(mpxe_gui
  ${APP_SOURCES}
  ${APP_HEADERS}      # just for IDEs; not required to build
)

# Public include dir for your headers (e.g., inc/overview_page.hpp, etc.)
target_include_directories(mpxe_gui PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/inc
  ${CMAKE_CURRENT_SOURCE_DIR}/src  # if you include headers from src
)

# ---- Qt linking ----
target_link_libraries(mpxe_gui PRIVATE
  Qt${QT_VERSION_MAJOR}::Core
  Qt${QT_VERSION_MAJOR}::Widgets
  Qt${QT_VERSION_MAJOR}::Network
)

# ---- Platform niceties ----
set_target_properties(mpxe_gui PROPERTIES
  WIN32_EXECUTABLE TRUE           # drop console window on Windows
  MACOSX_BUNDLE TRUE
)

# MSVC: ensure UTF‑8 source encoding (avoids warnings with QStringLiteral text)
if(MSVC)
  target_compile_options(mpxe_gui PRIVATE /utf-8)
endif()

# ---- Optional: a tiny smoke target ----
# Only link Qt here if your smoke sources include Qt headers.
# If it's pure C++ (iostream), you can omit the Qt link.
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/smoke.cpp")
  add_executable(mpxe_smoke
    src/smoke.cpp
    src/TestThing.cc
  )
  target_include_directories(mpxe_smoke PRIVATE inc src)
  # Uncomment if smoke uses Qt stuff:
  # target_link_libraries(mpxe_smoke PRIVATE
  #   Qt${QT_VERSION_MAJOR}::Core
  #   Qt${QT_VERSION_MAJOR}::Widgets
  # )
endif()
