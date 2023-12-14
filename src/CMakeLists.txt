include_directories(${CMAKE_CURRENT_SOURCE_DIR})
set(CPP_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_subdirectory(base)
add_subdirectory(algorithms)

file(GLOB_RECURSE SOURCES "*.c*")
file(GLOB_RECURSE HEADERS "*.h*")

add_library(EWD STATIC ${SOURCES} ${HEADERS})