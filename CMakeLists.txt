cmake_minimum_required(VERSION 3.18)

project ( SlimeMouldSimulation )

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m64 -std=c++17")
set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG} -Wall")

set (linker "-lopengl32 -lgdi32 -lmingw32 -luser32 -lSDL2main -lSDL2 -lSDL2_image -lSDL2_mixer -lSDL2_ttf")

set (source_dir "${PROJECT_SOURCE_DIR}/src")
set (CMAKE_BINARY_DIR "/bin")

set (EXECUTABLE_OUTPUT_PATH "${PROJECT_SOURCE_DIR}${CMAKE_BINARY_DIR}")

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set (EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/debug")
elseif (CMAKE_BUILD_TYPE MATCHES Release)
    set (EXECUTABLE_OUTPUT_PATH "${EXECUTABLE_OUTPUT_PATH}/release")
endif ()

if (WIN32)
    include_directories("libraries/GLAD/include")
    include_directories("libraries/SDL/include")
    include_directories("src/Vendor")

    link_directories("libraries/SDL/lib")
endif()

file (GLOB cppFiles CONFIGURE_DEPENDS "${source_dir}/*.cpp")
file (GLOB cFiles CONFIGURE_DEPENDS "${source_dir}/vendor/GLAD/*.c")

add_executable(${PROJECT_NAME} ${cppFiles} ${cFiles})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_SOURCE_DIR}/res" 
    "${EXECUTABLE_OUTPUT_PATH}/res"
    )

if (WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${linker})
endif()