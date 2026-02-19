add_library(senc_client_api SHARED
	"main.cpp"
	"client_api.hpp"
)

set_property(TARGET senc_client_api PROPERTY CXX_STANDARD 20)

# define I_SENC_CLIENT_API macro
target_compile_definitions(senc_client_api PRIVATE I_SENC_CLIENT_API)

# disable default function exporting on Linux
if(NOT WIN32)
	target_compile_options(senc_client_api PRIVATE -fvisibility=hidden)
endif()

target_link_libraries(senc_client_api PRIVATE
	senc_utils
	senc_common
)
