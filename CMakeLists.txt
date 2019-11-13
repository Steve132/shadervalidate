cmake_minimum_required(VERSION 3.10)

project(shadervalidate)
find_package(OpenCL REQUIRED)

add_executable(clvalidate 
	clvalidate.cpp
	clchooser.hpp
)
target_include_directories(clvalidate PRIVATE deps ${OpenCL_INCLUDE_DIRS})
target_link_libraries(clvalidate ${OpenCL_LIBRARIES})
set_target_properties(clvalidate PROPERTIES 
	CXX_STANDARD 11
	PUBLIC_HEADER clchooser.hpp
)
#target_compile_definitions(clvalidate PRIVATE -DCL_HPP_TARGET_OPENCL_VERSION=${OpenCL_VERSION_MAJOR}${OpenCL_VERSION_MINOR}0)
install(TARGETS clvalidate)
