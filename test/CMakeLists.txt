
add_executable(print_graph print_graph.cpp)
target_link_libraries(print_graph spaceflib fgraph)

add_executable(test_spacef test_main.cpp test_misc.cpp test_misc_size.cpp test_graph.cpp)
target_include_directories (spaceflib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(test_spacef spaceflib fgraph)

find_package(Boost COMPONENTS graph regex)
if(Boost_FOUND)
  target_include_directories(print_graph PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(print_graph ${Boost_LIBRARIES})
  target_compile_definitions(print_graph PRIVATE USE_BOOST=1)

  target_include_directories(test_spacef PUBLIC ${Boost_INCLUDE_DIRS})
  target_link_libraries(test_spacef ${Boost_LIBRARIES})
  target_compile_definitions(test_spacef PRIVATE USE_BOOST=1)
endif()
