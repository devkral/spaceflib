add_subdirectory (thirdparty)

add_library (spaceflib SHARED graph.cpp misc.cpp)
target_include_directories (spaceflib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(spaceflib sux_rank9sel)


add_library (fgraph SHARED fgraph.cpp)
target_include_directories (fgraph PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(fgraph spaceflib)
