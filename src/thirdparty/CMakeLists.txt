
#include(ExternalProject)
add_library(sux_rank9sel SHARED ./Sux/rank9sel.cpp)
target_include_directories (sux_rank9sel PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/Sux")
set_target_properties(sux_rank9sel PROPERTIES CMAKE_BUILD_TYPE MINSIZEREL)
