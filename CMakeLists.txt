cmake_minimum_required(VERSION 3.1)

project(spaceFlib)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# new CMAKE_BUILD_TYPE: Profile
SET(CMAKE_CXX_FLAGS_PROFILE "-DNDEBUG=1 -g -lprofiler -ltcmalloc -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
SET(CMAKE_C_FLAGS_PROFILE "-DNDEBUG=1 -g -lprofiler -ltcmalloc -fno-builtin-malloc -fno-builtin-calloc -fno-builtin-realloc -fno-builtin-free")
SET(CMAKE_EXE_LINKER_FLAGS_PROFILE "-g")


add_subdirectory (src)
add_subdirectory (test)
add_subdirectory (docs)
