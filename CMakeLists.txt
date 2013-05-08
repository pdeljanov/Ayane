#
# Stargazer Standard Audio Library (aka Ayane2)
#


# Add Boost and CppLog to the include directory search path.

include_directories( ${CPPLOG_INCLUDE_DIR} )


# Source files.

set( audio_SRCS
  buffer.cpp
  bufferformat.cpp
  bufferlength.cpp
  channels.cpp
  duration.cpp
  formatmismatchexception.cpp
  formats.cpp
  locklessqueue.cpp
  locklessrecyclingqueue.cpp
  logger.cpp
  rawaudiobuffer.cpp	)


# Header files (will be copied to system include directory).

set( audio_HDRS
  buffer.h
  bufferformat.h
  bufferlength.h
  channels.h
  duration.h
  formatmismatchexception.h
  formats.h
  locklessqueue.h
  locklessrecyclingqueue.h
  logger.h
  rawaudiobuffer.h	)


# Generate a library.

add_library( stargazer_stdlib_audio SHARED 
		${audio_SRCS} ${audio_HDRS} )


# Library installation information.

install( TARGETS stargazer_stdlib_audio
		LIBRARY DESTINATION lib/stargazer
		ARCHIVE DESTINATION lib/stargazer )


# Header installation information.

install( FILES ${audio_HDRS} 
		DESTINATION include/stargazer/audio )
