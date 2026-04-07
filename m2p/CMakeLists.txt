add_executable(m2p m2p.c util.c)

target_link_libraries(m2p PRIVATE md4c)
target_link_libraries(m2p PRIVATE libpdf)

# TODO: Better the build system, this is only a temporary hack to make CMake to only install the `m2p` binary
set(CMAKE_INSTALL_PREFIX ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY true)

install(
    TARGETS m2p
    COMPONENT m2p
    DESTINATION "${CMAKE_INSTALL_BINDIR}"
)
