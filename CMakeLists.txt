cmake_minimum_required(VERSION 3.5.1)

project(UDPServer VERSION 0.1.0 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_CXX_FLAGS "-g -Werror=unused-parameter")
set(CMAKE_CXX_STANDARD 17)


add_executable(udpclientest udpclientest.cpp udpclient.cpp)
add_executable(udpservertest udpservertest.cpp udpserver.cpp)
set(ROOT_ROOT ${CMAKE_SOURCE_DIR}/..)

#PRE BUILD COMMANDS 
get_filename_component(DISTRO_INCLUDE ${CMAKE_SOURCE_DIR}/dist/include  ABSOLUTE)
get_filename_component(DISTRO_LIBRARY ${CMAKE_SOURCE_DIR}/dist/lib      ABSOLUTE)


get_filename_component(INTERFACESUTIL_DISTRO_INCLUDE ${ROOT_ROOT}/networkinterfacesutil/dist/include ABSOLUTE)
get_filename_component(INTERFACESUTIL_DISTRO_LIBRARY ${ROOT_ROOT}/networkinterfacesutil/dist/lib ABSOLUTE)


get_filename_component(3RD_INCLUDE ${CMAKE_SOURCE_DIR}/3rd/include 	ABSOLUTE)
get_filename_component(3RD_LIBRARY ${CMAKE_SOURCE_DIR}/3rd/lib 		ABSOLUTE)



#LIBRARY LINK ARCHIVE & STUFF
add_library(UDPServer STATIC udpserver.cpp udpserver.h udpclient.cpp udpclient.h)


#POST BUILD COMMANDS

add_custom_command(TARGET UDPServer PRE_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy ${INTERFACESUTIL_DISTRO_LIBRARY}/* ${3RD_LIBRARY}
	COMMAND ${CMAKE_COMMAND} -E copy ${INTERFACESUTIL_DISTRO_INCLUDE}/* ${3RD_INCLUDE}
)


