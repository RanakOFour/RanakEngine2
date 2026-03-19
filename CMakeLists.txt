cmake_minimum_required(VERSION 3.20)
set(CMAKE_CXX_STANDARD 17)

project(RanakEngine)

# Try to find system libraries first, fallback to external versions
find_package(Lua QUIET)
find_package(GLEW QUIET)
find_package(SDL3 QUIET)
find_library(GL_LIBRARIES GL QUIET)

# Fallback for Lua
if(NOT Lua_FOUND)
    message(STATUS "Lua not found in system, checking extern...")
    add_subdirectory(extern/lua)
    set(LUA_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/lua)
endif()

message(STATUS "Lua includes: " ${LUA_INCLUDE_DIR})

# Fallback for SDL3
if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found in system, using extern/SDL3")
    set(SDL3_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/extern/SDL3)
endif()

file(GLOB_RECURSE RANAK_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/src/RanakEngine/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RanakEngine/Assets/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RanakEngine/Core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RanakEngine/IO/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RanakEngine/Log/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/RanakEngine/Math/*.cpp"
    )

file(GLOB_RECURSE RANAK_HEADER "${CMAKE_CURRENT_SOURCE_DIR}/include/RanakEngine/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/RanakEngine/Assets/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/RanakEngine/Core/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/RanakEngine/IO/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/RanakEngine/Log/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/include/RanakEngine/Math/*.h"
    )

add_library(RanakEngine STATIC ${RANAK_HEADER} ${RANAK_SOURCE})

target_include_directories(RanakEngine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/extern
    ${LUA_INCLUDE_DIR}
    ${GLEW_INCLUDE_DIRS}
    ${SDL3_INCLUDE_DIRS}
)

# Link against system libraries if found, otherwise extern
if(WIN32)
    target_link_libraries(RanakEngine PRIVATE opengl32)
    if(SDL3_FOUND)
        target_link_libraries(RanakEngine PRIVATE SDL3::SDL3)
    endif()
    if(Lua_FOUND)
        target_link_libraries(RanakEngine PRIVATE ${LUA_LIBRARIES})
    else()
        target_link_libraries(RanakEngine PRIVATE lua)
    endif()

    # Copy dlls from lib/win/bin to output dir
elseif(UNIX)
    if(GL_LIBRARIES)
        target_link_libraries(RanakEngine PRIVATE ${GL_LIBRARIES})
    endif()
    if(SDL3_FOUND)
        target_link_libraries(RanakEngine PRIVATE SDL3::SDL3)
    endif()
    if(Lua_FOUND)
        target_link_libraries(RanakEngine PRIVATE ${LUA_LIBRARIES})
    else()
        target_link_libraries(RanakEngine PRIVATE lua)
    endif()
endif()