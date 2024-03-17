set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED)

# specify the C++ standard
add_library(chat_app_compiler_flags INTERFACE)
target_compile_features(chat_app_compiler_flags INTERFACE cxx_std_17)

# add compiler warning flags just when building this project via
# the BUILD_INTERFACE genex
set(gcc_like_cxx "$<COMPILE_LANG_AND_ID:CXX,ARMClang,AppleClang,Clang,GNU,LCC>")
set(msvc_cxx "$<COMPILE_LANG_AND_ID:CXX,MSVC>")
target_compile_options(
  chat_app_compiler_flags INTERFACE
  "$<${gcc_like_cxx}:$<BUILD_INTERFACE:-Wall;-Wextra;-Wshadow;-Wformat=2;-Wunused;-O2;-g>>"
  "$<${msvc_cxx}:$<BUILD_INTERFACE:-W3;-O2>>"
)

# add the executable
add_executable(
  chat_app
  main.cpp
  tui.cpp
  net.cpp
  app.cpp
)

target_include_directories(
  chat_app
  PUBLIC ${CURSES_INCLUDE_DIR} 
  PUBLIC ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(
  chat_app 
  chat_app_compiler_flags
  ${CURSES_LIBRARIES}
)

set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/build)

# Install chat_app in the bin directory
install(
  TARGETS chat_app
  DESTINATION ${CMAKE_BINARY_DIR}
)
