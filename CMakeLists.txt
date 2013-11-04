##### Stargazer Standard Libraries -- Audio #####

### Sources ###

## Host ##

# Platform-dependant host sources #

if( ${STARGAZER_HOST} MATCHES "mac" )

	find_library(COREAUDIO_FRAMEWORK CoreAudio )
	find_library(AUDIO_TOOLBOX_FRAMEWORK AudioToolbox )
	find_library(AUDIO_UNIT_FRAMEWORK AudioUnit )

	set( audio_host_LIBS 
		${COREAUDIO_FRAMEWORK}
		${AUDIO_TOOLBOX_FRAMEWORK}
		${AUDIO_UNIT_FRAMEWORK}
		)

	set( audio_host_SRCS
		host/mac/coreaudioendpoint.cpp
		)
	
	set( audio_host_HDRS
		host/mac/coreaudioendpoint.h
		)

else()
	message( FATAL_ERROR "'stargazer_stdlib_audio' does not support your host." )
endif()

# Sources that are common to all hosts.
set( audio_host_shared_SRCS
	)
	
# Headers that are common to all hosts.
set( audio_host_shared_HDRS
	host/endpoint.h
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
	abstractclock.cpp
	abstractstage.cpp
	buffer.cpp
	bufferframes.cpp
	bufferformat.cpp
	bufferlength.cpp
	clock.cpp
	clockobserver.cpp
	clockprovider.cpp
	refcountedpool.cpp
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
	abstractclock.h
	abstractstage.h
	buffer.h
	bufferframes.h
	bufferformat.h
	bufferlength.h
	clock.h
	clockobserver.h
	clockprovider.h
	refcountedpool.h
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
							)
	
	install( TARGETS stargazer_stdlib_audio-pexe
			LIBRARY DESTINATION lib/stargazer
			ARCHIVE DESTINATION lib/stargazer 
			)
			
else()


	# Dynamic stdlib_audio library.
	add_library( stargazer_stdlib_audio SHARED
				${audio_SRCS} 
				${audio_trusted_SRCS}
				${audio_HDRS} 
				${audio_trusted_HDRS}
				
				# Host portion of library.
				${audio_host_SRCS}
				${audio_host_shared_SRCS}
				${audio_host_HDRS}
				${audio_host_shared_HDRS}
				
				# Host libraries
				${audio_host_LIBS}
				)
	
	target_link_libraries( stargazer_stdlib_audio
						 ${audio_host_LIBS}
							)
	

	# Test App.
	add_executable( stargazer_stdlib_audio-benchmark benchmark.cpp )
	
	target_link_libraries(	stargazer_stdlib_audio-benchmark
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
