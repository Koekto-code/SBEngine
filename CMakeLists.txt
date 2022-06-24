cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

project(SBEngine VERSION 0.1.0 LANGUAGES CXX)

option(EXTERNAL_GLFW_BUILD "Provide custom GLFW libraries" OFF)

if (NOT EXTERNAL_GLFW_BUILD)
	message(STATUS "Compiling GLFW from sources")
	add_subdirectory("deps/glfw")
	set_target_properties (glfw PROPERTIES
		ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
		RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
	)
	get_target_property(T_VAR_GLFW_OUTPUT_NAME glfw OUTPUT_NAME)
	set(VAR_GLFW_OUTPUT_NAME "${T_VAR_GLFW_OUTPUT_NAME}" CACHE INTERNAL "")
else()
	message(STATUS "Using external GLFW")
	set(EXTERNAL_GLFW_INCLUDE_DIR "" CACHE PATH "")
	set(EXTERNAL_GLFW_IMPORT_LIBRARY_DIR "" CACHE PATH "Specify the path to\n-l${VAR_GLFW_LIB_NAME}")
endif()
	
# Represent configs written in GLFW CMakeLists
if (GLFW_LIBRARY_TYPE)
	if (GLFW_LIBRARY_TYPE STREQUAL "SHARED")
		set(VAR_GLFW_SHARED_LIB TRUE)
	else()
		set(VAR_GLFW_SHARED_LIB FALSE)
	endif()
else()
	set(VAR_GLFW_SHARED_LIB ${BUILD_SHARED_LIBS})
endif()
if (VAR_GLFW_SHARED_LIB)
	if (WIN32)
		set(VAR_GLFW_IMPORT_SUFFIX "dll")
	endif()
endif()
set(VAR_GLFW_LIB_NAME "${VAR_GLFW_OUTPUT_NAME}${VAR_GLFW_IMPORT_SUFFIX}" CACHE INTERNAL "" FORCE)

set(EXTERNAL_ASSIMP_INCLUDE_DIR "" CACHE PATH "")
set(EXTERNAL_ASSIMP_IMPORT_LIBRARY_DIR "" CACHE PATH "Specify the path to\n-lassimp")

if (EXTERNAL_GLFW_BUILD AND EXTERNAL_GLFW_INCLUDE_DIR)
	include_directories("${EXTERNAL_GLFW_INCLUDE_DIR}")
else()
	include_directories("deps/glfw/include")
endif()
if (EXTERNAL_ASSIMP_INCLUDE_DIR)
	include_directories("${EXTERNAL_ASSIMP_INCLUDE_DIR}")
endif()
include_directories("deps/glad/include")
include_directories("deps/stb")
include_directories("deps/glm")

add_subdirectory(source)