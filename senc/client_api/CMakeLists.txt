add_library(senc_client_api SHARED
	"client_api.hpp"
	"client_api.cpp"
	"Handle.hpp"
	"Error.hpp"
	"Error.cpp"
	"Value.hpp"
	"Value_impl.hpp"
	"storage/ProfileRecord.hpp"
	"storage/ProfileRecord.cpp"
	"storage/ProfileStorage.hpp"
	"storage/ProfileStorage.cpp"
)

set_property(TARGET senc_client_api PROPERTY CXX_STANDARD 20)

# define I_SENC_CLIENT_API macro
target_compile_definitions(senc_client_api PRIVATE I_SENC_CLIENT_API)

# disable default function exporting on Linux
if(NOT WIN32)
	target_compile_options(senc_client_api PRIVATE -fvisibility=hidden)
endif()

set_target_properties(senc_utils PROPERTIES POSITION_INDEPENDENT_CODE ON)

target_link_libraries(senc_client_api PRIVATE
	senc_utils
	senc_common
)
