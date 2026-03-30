add_library(libpdf STATIC core.c ../include/libpdf.h)

include (GNUInstallDirs)

target_include_directories (libpdf 
    PUBLIC
        "$<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}/include>" 
        "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
)

target_link_libraries(libpdf PRIVATE hpdf)

