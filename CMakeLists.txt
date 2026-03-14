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
    # Lua needs to be found or compiled - adjust path if needed
    # For now, we assume if find_package fails, you'll need to provide it
endif()

# Fallback for GLEW
if(NOT GLEW_FOUND)
    message(STATUS "GLEW not found in system, using extern/GL")
    set(GLEW_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/extern/GL)
    # GLEW headers only - no linking needed if using header-only path
endif()

# Fallback for SDL3
if(NOT SDL3_FOUND)
    message(STATUS "SDL3 not found in system, using extern/SDL3")
    set(SDL3_INCLUDE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}/extern/SDL3)
endif()

add_library(RanakEngine STATIC)

add_subdirectory(src)

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
    if(GLEW_FOUND)
        target_link_libraries(RanakEngine PRIVATE GLEW::GLEW)
    endif()
    if(SDL3_FOUND)
        target_link_libraries(RanakEngine PRIVATE SDL3::SDL3)
    endif()
    if(Lua_FOUND)
        target_link_libraries(RanakEngine PRIVATE ${LUA_LIBRARIES})
    endif()
elseif(UNIX)
    if(GL_LIBRARIES)
        target_link_libraries(RanakEngine PRIVATE ${GL_LIBRARIES})
    endif()
    if(GLEW_FOUND)
        target_link_libraries(RanakEngine PRIVATE GLEW::GLEW)
    endif()
    if(SDL3_FOUND)
        target_link_libraries(RanakEngine PRIVATE SDL3::SDL3)
    endif()
    if(Lua_FOUND)
        target_link_libraries(RanakEngine PRIVATE ${LUA_LIBRARIES})
    endif()
endif()