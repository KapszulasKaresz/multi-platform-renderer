target_include_directories(${RENDERER_LIB_NAME} PUBLIC inc)

target_sources(${RENDERER_LIB_NAME} PRIVATE "src/D3D12MemAlloc.cpp")
