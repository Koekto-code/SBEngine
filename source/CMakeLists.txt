set (view_HEADERS
	common.hpp
	app/argparser.hpp
	app/callbacks.hpp
	app/graphics.hpp
	runtime/glUtil.h
	runtime/localMath.hpp
	runtime/camera.hpp
	runtime/mesh.hpp
	runtime/model.hpp
	runtime/sbObject.hpp
	runtime/shader.hpp
	runtime/print.hpp
	runtime/world.hpp
	models/skybox.h
	models/sph.h
)
set (view_SOURCES
	app/main.cpp
	app/argparser.cpp
	app/callbacks.cpp
	app/graphics.cpp
	../deps/glad/src/gl.c
	runtime/glUtil.c
	runtime/localMath.cpp
	runtime/camera.cpp
	runtime/mesh.cpp
	runtime/model.cpp
	runtime/sbObject.cpp
	runtime/shader.cpp
	runtime/stb_image.c
	runtime/print.cpp
	runtime/world.cpp
	models/sph.c
)
set_source_files_properties(${view_SOURCES} PROPERTIES LANGUAGE CXX)

add_executable(view ${view_SOURCES} ${view_HEADERS})
target_include_directories(view PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(view PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

set(glfwLibDirectory ${CMAKE_BINARY_DIR}/lib)
if (EXTERNAL_GLFW_BUILD)
	set(glfwLibDirectory ${EXTERNAL_GLFW_IMPORT_LIBRARY_DIR})
else()
	add_dependencies(view glfw)
endif()

set(assimpLinkDirectory ${CMAKE_BINARY_DIR}/lib)

target_link_directories(view PRIVATE  "${glfwLibDirectory}" "${EXTERNAL_ASSIMP_IMPORT_LIBRARY_DIR}")
target_link_libraries(view PRIVATE "${VAR_GLFW_LIB_NAME}" assimp)
