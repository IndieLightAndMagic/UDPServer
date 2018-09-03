cmake_minimum_required(VERSION 3.5.1)

set(TARGET_NAME UDPServer)
project(${TARGET_NAME} VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_VERBOSE_MAKEFILE 1)
set(CMAKE_CXX_FLAGS "-g -Werror=unused-parameter")
set(CMAKE_CXX_STANDARD 17)
set(ROOT_ROOT ${CMAKE_SOURCE_DIR}/..)


get_filename_component(3RD_INCLUDE ${CMAKE_SOURCE_DIR}/3rd/include ABSOLUTE)
get_filename_component(3RD_LIBRARY ${CMAKE_SOURCE_DIR}/3rd/lib ABSOLUTE)

get_filename_component(INTERFACESUTIL_DISTRO_INCLUDE ${ROOT_ROOT}/networkinterfacesutil/dist/include ABSOLUTE)
get_filename_component(INTERFACESUTIL_DISTRO_LIBRARY ${ROOT_ROOT}/networkinterfacesutil/dist/lib ABSOLUTE)
get_filename_component(SIGNALANDSLOTS_DISTRO_INCLUDE ${ROOT_ROOT}/signalslot/dist/include ABSOLUTE)


get_filename_component(DISTRO_INCLUDE ${CMAKE_SOURCE_DIR}/dist/include ABSOLUTE)
get_filename_component(DISTRO_LIBRARY ${CMAKE_SOURCE_DIR}/dist/lib ABSOLUTE)


#EXECUTABLES TO TEST LIBRARY SOURCES
add_executable(udpclientest udpclientest.cpp udpserver.cpp)
add_executable(udpservertest udpservertest.cpp udpserver.cpp ${3RD_LIBRARY}/libNetworkInterfacesUtil.a)

target_link_libraries(udpclientest ${3RD_LIBRARY}/libNetworkInterfacesUtil.a)
target_link_libraries(udpservertest ${3RD_LIBRARY}/libNetworkInterfacesUtil.a)


#LIBRARY LINK ARCHIVE & STUFF
add_library(${TARGET_NAME} STATIC udpserver.cpp udpserver.h udpclient.cpp udpclient.h)

#PRE BUILD COMMANDS 
FILE(GLOB 3RD_STATIC_LIBRARIES
	"${INTERFACESUTIL_DISTRO_LIBRARY}/*"
	)
FILE(GLOB 3RD_HEADER_INCLUDES
	"${INTERFACESUTIL_DISTRO_INCLUDE}/*.*"
    "${SIGNALANDSLOTS_DISTRO_INCLUDE}/*.*"
	)
FILE(COPY 
	${3RD_STATIC_LIBRARIES}
    DESTINATION ${3RD_LIBRARY}
)
FILE(COPY 
	${3RD_HEADER_INCLUDES}
    DESTINATION ${3RD_INCLUDE}
)

#INCLUDE DIRECTORIES 
include_directories(
    ${3RD_INCLUDE}
    )



#POST BUILD COMMANDS
file(MAKE_DIRECTORY ${DISTRO_INCLUDE})
add_custom_command(TARGET ${TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy lib${TARGET_NAME}.a ${DISTRO_LIBRARY}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_SOURCE_DIR}/*.h ${DISTRO_INCLUDE}
)

