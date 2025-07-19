cmake_minimum_required(VERSION 3.10)
project(QwenVulkanTokenizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Vulkan (via SDK)
find_package(Vulkan REQUIRED)

# Include nlohmann/json — header-only
find_path(NLOHMANN_JSON_INCLUDE_DIRS "nlohmann/json.hpp")
if(NOT NLOHMANN_JSON_INCLUDE_DIRS)
    message(FATAL_ERROR "nlohmann/json.hpp not found — install via 'sudo apt install nlohmann-json-dev'")
endif()

set(SOURCES
    src/main.cpp
    src/QwenTokenizer.cpp
    src/VulkanContext.cpp
    src/ShaderPipeline.cpp
    src/BufferHelper.cpp
    src/OutputReader.cpp
    src/DescriptorHelper.cpp
    src/DictionaryBuilder.cpp
    src/VulkanDispatcherHelper.cpp
)

add_executable(qwen_shader ${SOURCES})

target_include_directories(qwen_shader PRIVATE include ${NLOHMANN_JSON_INCLUDE_DIRS})
target_link_libraries(qwen_shader PRIVATE Vulkan::Vulkan)
