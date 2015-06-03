#
# Ayane Audio Library
#
# Copyright (c) 2013 Philip Deljanov <philip.deljanov@gmail.com>
#

cmake_minimum_required(VERSION 2.8)

# Make extra CMake modules available.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake" ${CMAKE_MODULE_PATH} )

# Include internal macros the extra CMake modules might need.
include(${CMAKE_MODULE_PATH}/MacroOptionalFindPackage.cmake)
include(${CMAKE_MODULE_PATH}/MacroEnsureVersion.cmake)

macro(add_prefix list_name prefix)
	set(${list_name}_TMP)

	foreach(l IN LISTS ${list_name})
		list(APPEND ${list_name}_TMP ${prefix}${l} )
	endforeach()

	set(${list_name} "${${list_name}_TMP}")
	unset(${list_name}_TMP)
endmacro()

# Ayane!
project(Ayane)

# Be strict (perhaps try -Weverything from time to time?).
add_definitions(-Wall -Wextra)
#add_definitions(-pedantic)

include_directories( "${CMAKE_SOURCE_DIR}/include" )

# Platform dependent sources

if(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

	# CoreAudio
	find_library(COREAUDIO_FRAMEWORK CoreAudio )
	find_library(AUDIO_TOOLBOX_FRAMEWORK AudioToolbox )
	find_library(AUDIO_UNIT_FRAMEWORK AudioUnit )

	set(ayane_host_SRCS src/Outputs/CoreAudioOutput.cxx )
	set(ayane_host_HDRS include/Ayane/Outputs/CoreAudioOutput.h)
	
	set(ayane_host_LIBS ${COREAUDIO_FRAMEWORK} ${AUDIO_TOOLBOX_FRAMEWORK} ${AUDIO_UNIT_FRAMEWORK})

	# C++11 support on Darwin.
	
	# Force XCode to use a C++11 standard library.
	if(${CMAKE_GENERATOR} MATCHES "Xcode")
		set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LANGUAGE_STANDARD "c++11" )
		set( CMAKE_XCODE_ATTRIBUTE_CLANG_CXX_LIBRARY "libc++" )
	endif()
	
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -stdlib=libc++")

elseif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

	# PulseAudio
	find_package(PulseAudio)

	if(PULSEAUDIO_FOUND)
		set(ayane_host_SRCS src/Outputs/PulseAudioOutput.cxx)
		set(ayane_host_HDRS include/Ayane/Outputs/PulseAudioOutput.h)
		
		set(ayane_host_LIBS ${PULSEAUDIO_LIBRARY})
		
		include_directories(${PULSEAUDIO_INCLUDE_DIR}})
	endif()

	# C++11 support on Linux.
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	
elseif(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
	message( FATAL_ERROR "Ayane does not support Windows yet." )

else()
	message( FATAL_ERROR "Ayane does not support your build target." )
	
endif()

# Platform independent sources.
set(ayane_SRCS
	AlignedMemory.cxx
	Buffer.cxx
	BufferFactory.cxx
	BufferFrames.cxx
	BufferFormat.cxx
	BufferLength.cxx
	BufferPool.cxx
	BufferQueue.cxx
	Clock.cxx
	ClockProvider.cxx
	Channels.cxx
	Duration.cxx
	SampleFormats.cxx
	MessageBus.cxx
	Pipeline.cxx
	RawBuffer.cxx
	Stage.cxx
	Trace.cxx
	)

set(ayane_HDRS
	AlignedMemory.h
	Attributes.h
	Buffer.h
	BufferFactory.h
	BufferFrames.h
	BufferFormat.h
	BufferLength.h
	BufferPool.h
	BufferQueue.h
	Channels.h
	Clock.h
	ClockProvider.h
	Duration.h
	DPointer.h
	SampleFormats.h
	Macros.h
	MessageBus.h
	Pipeline.h
	RawBuffer.h
	Stage.h
	Trace.h
	)

add_prefix(ayane_SRCS "src/")
add_prefix(ayane_HDRS "include/Ayane/")

# Ayane library build target.
add_library(Ayane SHARED
		# Platform independent sources
		${ayane_SRCS} 
		${ayane_HDRS} 

		# Host dependent sources
		${ayane_host_SRCS}
		${ayane_host_HDRS}

		# Host libraries (for frameworks)
		${ayane_host_LIBS}
		)

target_link_libraries(Ayane ${ayane_host_LIBS})

install(TARGETS Ayane
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
	)

install(DIRECTORY "${CMAKE_SOURCE_DIR}/include/Ayane" DESTINATION include)
