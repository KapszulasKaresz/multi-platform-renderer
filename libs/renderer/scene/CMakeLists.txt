target_sources(${RENDERER_LIB_NAME} PRIVATE
    "src/test_scene.cpp")

add_subdirectory(camera)
add_subdirectory(node)
add_subdirectory(node_visitor)
