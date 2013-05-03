#
# Stargazer Standard Audio Library (aka Ayane)
#


# Add Boost and CppLog to the include directory search path.

include_directories( ${CPPLOG_INCLUDE_DIR} )


# Source files.

set( audio_SRCS
  atomicoperations.cpp
  audiobuffer.cpp
  audiobufferlength.cpp
  asynchronousadaptor.cpp
  abstractaudiosink.cpp
  abstractaudiosource.cpp
  abstractaudiodsp.cpp
  abstractendpoint.cpp
  channels.cpp
  formats.cpp
  formatmismatchexception.cpp
  memoryoperations.cpp
  audiobufferformat.cpp
  duration.cpp
  nosourceexception.cpp
  invalidstateexception.cpp
  locklessqueue.cpp
  locklessrecyclingqueue.cpp
  logger.cpp
  result.cpp
  rawaudiobuffer.cpp
  sampleformatconverters.cpp
  supportedaudiobufferformats.cpp	)


# Header files (will be copied to system include directory).

set( audio_HDRS
  atomicoperations.h
  audiobuffer.h
  audiobufferlength.h
  asynchronousadaptor.h
  abstractaudiosink.h
  abstractaudiosource.h
  abstractaudiodsp.h
  abstractendpoint.h
  channels.h
  formats.h
  formatmismatchexception.h
  memoryoperations.h
  audiobufferformat.h
  duration.h
  nosourceexception.h
  invalidstateexception.h
  locklessqueue.h
  locklessrecyclingqueue.h
  logger.h
  result.h
  rawaudiobuffer.h
  sampleformatconverters.h
  supportedaudiobufferformats.h	)


# Generate a library.

add_library( stargazer_audio SHARED 
		${audio_SRCS} ${audio_HDRS} )


# Library installation information.

install( TARGETS stargazer_audio
		LIBRARY DESTINATION lib/stargazer
		ARCHIVE DESTINATION lib/stargazer )


# Header installation information.

install( FILES ${audio_HDRS} 
		DESTINATION include/stargazer/audio )
