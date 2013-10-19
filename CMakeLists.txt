##### Stargazer Standard Libraries -- Audio #####

### Sources ###

## Host ##

# Platform-dependant host sources #

if( ${STARGAZER_HOST} MATCHES "mac" )

	find_library(COREAUDIO_LIBRARY CoreAudio )

	set( audio_host_LIBS ${COREAUDIO_LIBRARY} )

	set( audio_host_SRCS
		host/mac/endpoint.cpp
		)
	
	set( audio_host_HDRS
		host/endpoint.h
		)

else()
	message( FATAL_ERROR "'stargazer_stdlib_audio' does not support your host." )
endif()

# Sources that are common to all hosts.
set( audio_host_shared_SRCS
	)
	
# Headers that are common to all hosts.
set( audio_host_shared_HDRS
	)

## Interface ##

# Sources used only in untrusted builds of audio.
set( audio_untrusted_SRCS
	)
	
# Headers used only in untrusted builds of audio.
set( audio_untrusted_HDRS
	)

	
# Sources used only in trusted builds of audio.
set( audio_trusted_SRCS
	)

# Headers used only in trusted builds of audio.
set( audio_trusted_HDRS
	)


# Sources common to both trusted and untrusted builds of audio.
set( audio_SRCS
	abstractclocksource.cpp
	abstractstage.cpp
	buffer.cpp
	bufferframes.cpp
	bufferformat.cpp
	bufferlength.cpp
	channels.cpp
	duration.cpp
	formatmismatchexception.cpp
	formats.cpp
	messagebus.cpp
	pipeline.cpp
	rawbuffer.cpp
  	)

# Headers common to both trusted and untrusted builds of audio.
set( audio_HDRS
	abstractclocksource.h
	abstractstage.h
	buffer.h
	bufferframes.h
	bufferformat.h
	bufferlength.h
	channels.h
	duration.h
	formatmismatchexception.h
	formats.h
	messagebus.h
	pipeline.h
	rawbuffer.h
	)


### Targets ###


if( STARGAZER_IS_PLATFORM )


	add_library( stargazer_stdlib_audio-pexe STATIC
			${audio_SRCS} 
			${audio_untrusted_SRCS}
			${audio_HDRS} 
			${audio_untrusted_HDRS}
			)
		
	target_link_libraries( stargazer_stdlib_audio-pexe 
							stargazer_stdlib_core-pexe
							)
	
	install( TARGETS stargazer_stdlib_audio-pexe
			LIBRARY DESTINATION lib/stargazer
			ARCHIVE DESTINATION lib/stargazer 
			)
			
else()

	add_library( stargazer_stdlib_audio-host STATIC
				${audio_host_SRCS}
				${audio_host_shared_SRCS}
				${audio_host_HDRS}
				${audio_host_shared_HDRS}
				${audio_host_LIBS}
				)

	target_link_libraries( stargazer_stdlib_audio-host
							${audio_host_LIBS}
							)
		
	add_library( stargazer_stdlib_audio SHARED
				${audio_SRCS} 
				${audio_trusted_SRCS}
				${audio_HDRS} 
				${audio_trusted_HDRS}
				)

	target_link_libraries( stargazer_stdlib_audio
							stargazer_stdlib_audio-host 
                            stargazer_stdlib_core
							)

	add_executable( stargazer_stdlib_audio-benchmark benchmark.cpp )
	target_link_libraries( stargazer_stdlib_audio-benchmark
		stargazer_stdlib_audio
		)

	install( TARGETS stargazer_stdlib_audio
			LIBRARY DESTINATION lib/stargazer
			ARCHIVE DESTINATION lib/stargazer 
			)
	
endif()


### Header Installation ###


install( FILES ${audio_HDRS}
	DESTINATION "${libraries_HEADER_INSTALL_DIR}/audio" )
