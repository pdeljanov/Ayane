#
# Stargazer Standard Audio Library (aka Ayane2)
#


# Add CppLog to the include directory search path.

include_directories( ${CPPLOG_INCLUDE_DIR} )

# Add in the core library.

include_directories( "../core/" )

# Source files.

set( audio_SRCS
  buffer.cpp
  bufferformat.cpp
  bufferlength.cpp
  channels.cpp
  duration.cpp
  formatmismatchexception.cpp
  formats.cpp
  logger.cpp
  rawbuffer.cpp	)


# Header files (will be copied to system include directory).

set( audio_HDRS
  buffer.h
  bufferformat.h
  bufferlength.h
  channels.h
  duration.h
  formatmismatchexception.h
  formats.h
  logger.h
  rawbuffer.h	)


# Generate a library.

add_library( stargazer_stdlib_audio 
		${audio_SRCS} ${audio_HDRS} )

target_link_libraries( stargazer_stdlib_audio stargazer_stdlib_core )

# Library installation information.

install( TARGETS stargazer_stdlib_audio
		LIBRARY DESTINATION lib/stargazer
		ARCHIVE DESTINATION lib/stargazer )


# Header installation information.

install( FILES ${audio_HDRS} 
		DESTINATION include/stargazer/audio )
