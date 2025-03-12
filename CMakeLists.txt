cmake_minimum_required(VERSION 3.10)

project(lasvk)

find_package(glfw3 REQUIRED)
find_package(Vulkan REQUIRED)
find_package(glm REQUIRED)
find_package(Stb REQUIRED)
find_package(SPNG CONFIG REQUIRED)
find_package(PCL REQUIRED)

include_directories(${PCL_INCLUDE_DIRS})
link_directories(${PCL_LIBRARY_DIRS})
add_definitions(${PCL_DEFINITIONS})

add_executable(lasvk)

target_sources(lasvk
PRIVATE
src/laser/laser.cpp
src/lvx_file/lvx_file.cpp
src/vkcam/vkcam.cpp
src/vkmesh/vkmesh.cpp
src/ransac/ransac.cpp
src/pcd_file/pcd_file.cpp
src/vkdev/vkdev.cpp
src/objreader/simple_obj_reader.cpp
src/imgui/imgui_tables.cpp
src/imgui/backends/imgui_impl_glfw.cpp
src/imgui/backends/imgui_impl_vulkan.cpp
src/imgui/imgui_widgets.cpp
src/imgui/imgui_draw.cpp
src/imgui/imgui.cpp
src/imgui/imgui_demo.cpp
src/main.cpp
src/vkrun/vkrun.cpp
src/las_file/las_file.cpp
src/vkgui/vkgui.cpp
)

target_include_directories(lasvk PUBLIC src)
target_include_directories(lasvk PUBLIC src/imgui)
target_include_directories(lasvk PUBLIC ${Stb_INCLUDE_DIR})

target_link_libraries(lasvk ${PCL_LIBRARIES} glfw Vulkan::Vulkan glm::glm)
target_link_libraries(lasvk $<IF:$<TARGET_EXISTS:spng::spng>,spng::spng,spng::spng_static>)

add_custom_target(copy_shaders COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/shaders ${CMAKE_CURRENT_BINARY_DIR}/shaders)

add_dependencies(lasvk copy_shaders)

add_compile_options(-fsanitize=address)