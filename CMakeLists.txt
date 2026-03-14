cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

project(RanakEngine)

find_package(Lua REQUIRED)
find_package(GLEW REQUIRED)
find_package(SDL3 REQUIRED)
find_library(GL_LIBRARIES GL)

add_library(RanakEngine STATIC)

add_subdirectory(src)

target_include_directories(RanakEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${LUA_INCLUDE_DIR}
)

if(WIN32)
    #target_link_directories(RanakEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/windows)
    target_link_libraries(RanakEngine PRIVATE opengl32 GLEW SDL3 ${LUA_LIBRARIES})
elseif(UNIX)
    #target_link_directories(RanakEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib/unix)
    target_link_libraries(RanakEngine PRIVATE ${GL_LIBRARIES} GLEW SDL3 ${LUA_LIBRARIES})
endif()