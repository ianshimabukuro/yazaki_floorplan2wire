# Use latest UseSWIG module (3.14) and Python3 module (3.18)
cmake_minimum_required(VERSION 3.18)

# Find Python 3
# 64位
if(NOT Python3_ROOT_DIR)
    if (CMAKE_CL_64)
        set(Python3_ROOT_DIR "C:/Program Files (x86)/Microsoft Visual Studio/Shared/Python37_64/")
    # 32
    else()
        set(Python3_ROOT_DIR "C:/Program Files (x86)/Microsoft Visual Studio/Shared/Python37_86/")
    endif()
endif()

# Will need swig
set(CMAKE_SWIG_FLAGS)
find_package(SWIG REQUIRED)
include(UseSWIG)

if(${SWIG_VERSION} VERSION_GREATER_EQUAL 4)
    list(APPEND CMAKE_SWIG_FLAGS "-doxygen")
endif()

if(UNIX AND NOT APPLE)
    list(APPEND CMAKE_SWIG_FLAGS "-DSWIGWORDSIZE64")
endif()


find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)
list(APPEND CMAKE_SWIG_FLAGS "-py3" "-DPY3")

# Find if the python module is available,
# otherwise install it (PACKAGE_NAME) to the Python3 user install directory.
# If CMake option FETCH_PYTHON_DEPS is OFF then issue a fatal error instead.
# e.g
# search_python_module(
#   NAME
#     mypy_protobuf
#   PACKAGE
#     mypy-protobuf
#   NO_VERSION
# )
function(search_python_module)
    set(options NO_VERSION)
    set(oneValueArgs NAME PACKAGE)
    set(multiValueArgs "")
    cmake_parse_arguments(MODULE
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )
    message(STATUS "Searching python module: \"${MODULE_NAME}\"")
    if(${MODULE_NO_VERSION})
        execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import ${MODULE_NAME}"
        RESULT_VARIABLE _RESULT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        set(MODULE_VERSION "unknown")
    else()
        execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import ${MODULE_NAME}; print(${MODULE_NAME}.__version__)"
        RESULT_VARIABLE _RESULT
        OUTPUT_VARIABLE MODULE_VERSION
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
    if(${_RESULT} STREQUAL "0")
        message(STATUS "Found python module: \"${MODULE_NAME}\" (found version \"${MODULE_VERSION}\")")
    else()
        if(FETCH_PYTHON_DEPS)
        message(WARNING "Can't find python module: \"${MODULE_NAME}\", install it using pip...")
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -m pip install --user ${MODULE_PACKAGE}
            OUTPUT_STRIP_TRAILING_WHITESPACE
            )
        else()
        message(FATAL_ERROR "Can't find python module: \"${MODULE_NAME}\", please install it using your system package manager.")
        endif()
    endif()
endfunction()


# 使用单个.i 文件 生成 python target 函数， 读取  SOLUTION_NAME.i 文件， 链接 SOLUTION_NAME 生成 py_SOLUTION_NAME python 目标，
# 默认include 目录 ${Python3_INCLUDE_DIRS} "../include" ".." ${PROJECT_SOURCE_DIR} 
function(swig_add_single_file_python SOLUTION_NAME)
    set(PYTHON_TARGET_NAME ${SOLUTION_NAME}py)
    set_property(SOURCE ${SOLUTION_NAME}.i PROPERTY CPLUSPLUS ON)
    set_property(SOURCE ${SOLUTION_NAME}.i PROPERTY SWIG_MODULE_NAME ${PYTHON_TARGET_NAME})
    swig_add_library(${PYTHON_TARGET_NAME}
        TYPE SHARED
        LANGUAGE python
        OUTPUT_DIR ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
        SOURCES ${SOLUTION_NAME}.i)

    set_property(TARGET ${PYTHON_TARGET_NAME} PROPERTY SWIG_USE_TARGET_INCLUDE_DIRECTORIES ON)

    add_custom_command(
        TARGET ${PYTHON_TARGET_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E rm "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${SOLUTION_NAME}PYTHON_wrap.cxx"
        COMMAND echo "finish remove  ${SOLUTION_NAME}PYTHON_wrap.cxx"    
    )
    target_include_directories(${PYTHON_TARGET_NAME} PRIVATE ${Python3_INCLUDE_DIRS} "../include" ".." ${PROJECT_SOURCE_DIR})
    target_link_libraries(${PYTHON_TARGET_NAME} PRIVATE ${SOLUTION_NAME})
    target_compile_definitions(${PYTHON_TARGET_NAME} PUBLIC "PY3")
    target_link_libraries(${PYTHON_TARGET_NAME} PRIVATE ${Python3_LIBRARIES})
endfunction()

# Find if a python builtin module is available.
# e.g
# search_python_internal_module(
#   NAME
#     mypy_protobuf
# )
function(search_python_internal_module)
    set(options "")
    set(oneValueArgs NAME)
    set(multiValueArgs "")
    cmake_parse_arguments(MODULE
        "${options}"
        "${oneValueArgs}"
        "${multiValueArgs}"
        ${ARGN}
    )
    message(STATUS "Searching python module: \"${MODULE_NAME}\"")
    execute_process(
        COMMAND ${Python3_EXECUTABLE} -c "import ${MODULE_NAME}"
        RESULT_VARIABLE _RESULT
        ERROR_QUIET
        OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    if(${_RESULT} STREQUAL "0")
        message(STATUS "Found python internal module: \"${MODULE_NAME}\"")
    else()
        message(FATAL_ERROR "Can't find python internal module \"${MODULE_NAME}\", please install it using your system package manager.")
    endif()
endfunction()

set(PYTHON_PROJECT EWDPY)
message(STATUS "Python project: ${PYTHON_PROJECT}")
set(PYTHON_PROJECT_DIR ${CMAKE_CURRENT_BINARY_DIR}/bin/python/${PYTHON_PROJECT})
message(STATUS "Python project build path: ${PYTHON_PROJECT_DIR}")

# Swig wrap all libraries
add_subdirectory(src/python)
# add_python_example()
# CMake function to generate and build python example.
# Parameters:
#  the python filename
# e.g.:
# add_python_example(foo.py)
function(add_python_example FILE_NAME)
    message(STATUS "Configuring example ${FILE_NAME} ...")
    get_filename_component(EXAMPLE_NAME ${FILE_NAME} NAME_WE)

    if(BUILD_TESTING)
        add_test(
        NAME python_example_${EXAMPLE_NAME}
        COMMAND ${VENV_Python3_EXECUTABLE} ${FILE_NAME}
        WORKING_DIRECTORY ${VENV_DIR})
    endif()
    message(STATUS "Configuring example ${FILE_NAME} done")
endfunction()
