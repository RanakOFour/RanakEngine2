file(GLOB RANAKENGINE_SOURCES 
    "*.cpp"
    "Core/*.cpp"
    "Math/*.cpp"
    "IO/*.cpp"
    "Log/*.cpp"
    "Asset/*.cpp"
)

target_sources(RanakEngine PUBLIC ${RANAKENGINE_SOURCES})