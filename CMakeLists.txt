CMAKE_MINIMUM_REQUIRED(VERSION 4.0)

project(shadowDelve LANGUAGES CXX C)

file(GLOB_RECURSE PROJECT_SOURCES CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
)

file(GLOB_RECURSE PROJECT_HEADERS CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp"
)

add_executable(shadowDelve ${PROJECT_SOURCES} ${PROJECT_HEADERS})
target_include_directories(shadowDelve PRIVATE src)
target_include_directories(shadowDelve PRIVATE include)
target_include_directories(shadowDelve PRIVATE vireon/vireon.hpp)

target_compile_features(shadowDelve PRIVATE cxx_std_20)
add_subdirectory(vireon)
target_link_libraries(shadowDelve vireon)

set_target_properties(shadowDelve PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

