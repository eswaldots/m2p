message(STATUS "Fetching libharu...")

# No need to build audio and network modules
set(BUILD_SHARED_LIBS FALSE)

FetchContent_MakeAvailable(libharu)
