cmake_minimum_required(VERSION 3.7 FATAL_ERROR)

project(CellModeller5)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")

# Set build output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

if(APPLE)
	set(USE_VULKAN_COMPAT_MODE ON)
else()
	set(USE_VULKAN_COMPAT_MODE OFF)
endif()

############ Setup Vulkan-Headers ############
set(VK_HEADERS_INCLUDE "${PROJECT_SOURCE_DIR}/third_party/Vulkan-Headers/include")

add_library(Vulkan-Headers INTERFACE)
target_include_directories(Vulkan-Headers INTERFACE ${VK_HEADERS_INCLUDE})

############ Setup volk ############
set(VOLK_ROOT "${PROJECT_SOURCE_DIR}/third_party/volk")

add_library(volk ${VOLK_ROOT}/volk.h ${VOLK_ROOT}/volk.c)

target_include_directories(volk PUBLIC ${VOLK_ROOT}/)
target_link_libraries(volk Vulkan-Headers)

if(USE_VULKAN_COMPAT_MODE)
	target_compile_definitions(volk PUBLIC VK_ENABLE_BETA_EXTENSIONS)
endif()

# set_target_properties(volk PROPERTIES LINKER_LANGUAGE CXX)

########### Setup glslang ############
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(SKIP_GLSLANG_INSTALL ON CACHE BOOL "" FORCE)
set(ENABLE_SPVREMAPPER OFF CACHE BOOL "")
set(ENABLE_GLSLANG_BINARIES OFF CACHE BOOL "")
set(ENABLE_CTEST OFF CACHE BOOL "")
set(ENABLE_HLSL OFF CACHE BOOL "")

add_subdirectory(third_party/glslang)

############ Setup pynind11 ############
set(PYBIND11_INSTALL OFF CACHE BOOL "" FORCE)
set(PYBIND11_TEST OFF CACHE BOOL "" FORCE)

add_subdirectory(third_party/pybind11)

############ Setup zlib ############
add_subdirectory(third_party)

############ Setup sources ############
if (WIN32)
   set(GLOBAL_DEFINES VK_USE_PLATFORM_WIN32_KHR)
elseif()
   # Other platforms
endif()

file(GLOB_RECURSE SOURCE_FILES "${PROJECT_SOURCE_DIR}/native/*.cpp" "${PROJECT_SOURCE_DIR}/native/*.h")

if (NOT CELLMODELLER_PYTHON_MODULE_NAME)
	set(CELLMODELLER_PYTHON_MODULE_NAME "native")
endif()

if (NOT CELLMODELLER_ARTIFACT_NAME)
	set(CELLMODELLER_ARTIFACT_NAME "native")
endif()

if (NOT CELLMODELLER_ARTIFACT_SUFFIX)
	set(CELLMODELLER_ARTIFACT_SUFFIX ".pyd")
endif()

if(MSVC)
	add_definitions(/MP)
endif()

# Just so we don't have to type the whole name every time
set(MODULE_NAME ${CELLMODELLER_PYTHON_MODULE_NAME})

pybind11_add_module(${MODULE_NAME} ${SOURCE_FILES})

include_directories("${PROJECT_SOURCE_DIR}/src/")

target_link_libraries(${MODULE_NAME} PRIVATE Vulkan-Headers)
target_link_libraries(${MODULE_NAME} PRIVATE volk)
target_link_libraries(${MODULE_NAME} PRIVATE glslang)
target_link_libraries(${MODULE_NAME} PRIVATE SPIRV)
target_link_libraries(${MODULE_NAME} PRIVATE zlibstatic)

target_include_directories(${MODULE_NAME} PUBLIC src)

target_compile_definitions(${MODULE_NAME} PRIVATE ${GLOBAL_DEFINES})
target_compile_definitions(${MODULE_NAME} PRIVATE CM_MODULE_NAME=${MODULE_NAME})

if(USE_VULKAN_COMPAT_MODE)
	target_compile_definitions(${MODULE_NAME} PRIVATE CM_VULKAN_COMPAT=1)
endif()

if(USE_DEBUG_DIALOG)
	target_compile_definitions(${MODULE_NAME} PRIVATE CM_WITH_DEBUG_DIALOG)
endif()

if(USE_RENDERDOC_CAPTURE)
	target_compile_definitions(${MODULE_NAME} PRIVATE CM_WITH_FRAME_CAPTURE)
endif()

set_target_properties(${MODULE_NAME} PROPERTIES OUTPUT_NAME ${CELLMODELLER_ARTIFACT_NAME})
set_target_properties(${MODULE_NAME} PROPERTIES SUFFIX ${CELLMODELLER_ARTIFACT_SUFFIX})

############ Setup IDE filters ############
macro(ConvertToFilters curdir)
	file(GLOB children RELATIVE ${PROJECT_SOURCE_DIR}/${curdir} ${PROJECT_SOURCE_DIR}/${curdir}/*)
	foreach(child ${children})
		if(IS_DIRECTORY ${PROJECT_SOURCE_DIR}/${curdir}/${child})
			ConvertToFilters(${curdir}/${child})
		else()
			string(REPLACE "/" "\\" groupname ${curdir})
			
			source_group(${groupname} FILES ${PROJECT_SOURCE_DIR}/${curdir}/${child})
		endif()
	endforeach()
endmacro()

ConvertToFilters(data)
ConvertToFilters(src)