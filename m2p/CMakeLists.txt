add_executable(m2p m2p.c util.c)

target_link_libraries(m2p PRIVATE md4c)
target_link_libraries(m2p PRIVATE libpdf)
