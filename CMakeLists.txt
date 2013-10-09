##### Stargazer Standard Libraries -- Audio #####

include_directories( "../core/" )

### Sources ###


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
	buffer.cpp
	bufferformat.cpp
	bufferlength.cpp
	channels.cpp
	duration.cpp
	formatmismatchexception.cpp
	formats.cpp
	logger.cpp
	rawbuffer.cpp
  	)

# Headers common to both trusted and untrusted builds of audio.
set( audio_HDRS
	buffer.h
	bufferformat.h
	bufferlength.h
	channels.h
	duration.h
	formatmismatchexception.h
	formats.h
	logger.h
	rawbuffer.h
	)


### Targets ###


if( ${STARGAZER_ARCH} MATCHES "pexe" )

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
			ARCHIVE DESTINATION lib/stargazer )
			
else()

	add_library( stargazer_stdlib_audio SHARED
			${audio_SRCS} 
			${audio_trusted_SRCS}
			${audio_HDRS} 
			${audio_trusted_HDRS}
			)
		
	target_link_libraries( stargazer_stdlib_audio 
		stargazer_stdlib_core 
		)

	install( TARGETS stargazer_stdlib_audio
			LIBRARY DESTINATION lib/stargazer
			ARCHIVE DESTINATION lib/stargazer )
	
endif()


### Header Installation ###


install( FILES ${audio_HDRS}
	DESTINATION "${libraries_HEADER_INSTALL_DIR}/core" )
