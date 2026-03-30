cmake_minimum_required(VERSION 3.15)

project(libpdf VERSION 0.1
                  DESCRIPTION "pdf library to generate markdown-like objects inside a pdf"
		  LANGUAGES C)

add_subdirectory(deps)
add_subdirectory(libpdf)
add_subdirectory(m2p)

