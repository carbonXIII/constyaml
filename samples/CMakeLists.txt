add_executable(config config.cpp)
target_include_directories(config PUBLIC ${CMAKE_SOURCE_DIR})
target_link_libraries(config PUBLIC constyaml)
