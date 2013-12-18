#
# Ayane Audio Library
#
# Copyright (c) 2013 Philip Deljanov <philip.deljanov@gmail.com>
#

cmake_minimum_required(VERSION 2.8)

project(Ayane)

# C++11

# Force XCode to use C++11 since it ignores CXX flags.
if( ${CMAKE_GENERATOR} MATCHES "Xcode" )
	set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11" )
	set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
endif()

# Enable C++11 support. Use libc++ for C++11 support.
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++" )

# Be strict (perhaps try -Weverything from time to time?).
add_definitions(-Wall
				-Wextra
				)

#add_definitions( -pedantic )

# Include source directory in includes so we can use absolute paths to include headers
include_directories( ${CMAKE_SOURCE_DIR} )


# Platform dependent sources

if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
			
	find_library(COREAUDIO_FRAMEWORK CoreAudio )
	find_library(AUDIO_TOOLBOX_FRAMEWORK AudioToolbox )
	find_library(AUDIO_UNIT_FRAMEWORK AudioUnit )

	set( ayane_host_LIBS 
		${COREAUDIO_FRAMEWORK}
		${AUDIO_TOOLBOX_FRAMEWORK}
		${AUDIO_UNIT_FRAMEWORK}
		)

	set( ayane_host_SRCS
		host/mac/coreaudioendpoint.cpp
		)
	
	set( ayane_host_HDRS
		host/mac/coreaudioendpoint.h
		)
		
elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
	message( FATAL_ERROR "Ayane does not support Linux yet." )

elseif( ${CMAKE_SYSTEM_NAME} MATCHES "Windows" )
	message( FATAL_ERROR "Ayane does not support Windows yet." )

else()
	message( FATAL_ERROR "Ayane does not support your build target." )
	
endif()

# Platform independent sources

set(ayane_SRCS
    alignedmemory.cpp
	buffer.cpp
	bufferfactory.cpp
	bufferframes.cpp
	bufferformat.cpp
	bufferlength.cpp
    bufferpool.cpp
	bufferqueue.cpp
	clock.cpp
	clockprovider.cpp
	channels.cpp
	duration.cpp
	formats.cpp
	messagebus.cpp
	pipeline.cpp
	rawbuffer.cpp
	stage.cpp
    trace.cpp
  	)

set(ayane_HDRS
    alignedmemory.h
    attributes.h
	buffer.h
	bufferfactory.h
	bufferframes.h
	bufferformat.h
	bufferlength.h
    bufferpool.h
	bufferqueue.h
	clock.h
	clockprovider.h
    dpointer.h
	channels.h
	duration.h
	formats.h
    macros.h
	messagebus.h
	pipeline.h
	rawbuffer.h
	stage.h
    trace.h
	)

set(ayane_host_common_SRCS
	)
	
set(ayane_host_common_HDRS
	host/endpoint.h
	)

### Targets ###

add_library( ayane SHARED
			${ayane_SRCS} 
			${ayane_HDRS} 
			
			# Host portion of library.
			${ayane_host_SRCS}
			${ayane_host_common_SRCS}
			${ayane_host_HDRS}
			${ayane_host_common_HDRS}
			
			# Host libraries
			${ayane_host_LIBS}
			)

target_link_libraries(ayane ${ayane_host_LIBS})

install(TARGETS ayane
		LIBRARY DESTINATION lib
		ARCHIVE DESTINATION lib
		)