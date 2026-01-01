add_library(senc_utils
	"env.hpp"
	"bytes.hpp"
	"bytes.cpp"
	"strs.hpp"
	"bytes_impl.hpp"
	"concepts.hpp"
	"hash.hpp"
	"variants.hpp"
	"ranges.hpp"
	"ranges_impl.hpp"
	"math.hpp"
	"Exception.hpp"
	"Exception.cpp"
	"StrParseException.hpp"
	"Socket.hpp"
	"Socket.cpp"
	"Socket_impl.hpp"
	"poly.hpp"
	"poly_impl.hpp"
	"Random.hpp"
	"Random_impl.hpp"
	"ModInt.hpp"
	"ModInt_impl.hpp"
	"Fraction.hpp"
	"Fraction_impl.hpp"
	"Group.hpp"
	"ECGroup.hpp"
	"ECGroup.cpp"
	"enc/general.hpp"
	"enc/HybridElGamal2L.hpp"
	"enc/HybridElGamal2L_impl.hpp"
	"enc/AES1L.hpp"
	"enc/AES1L.cpp"
	"enc/ECHKDF2L.hpp"
	"enc/ECHKDF2L.cpp"
	"Shamir.hpp"
	"Shamir_impl.hpp"
	"uuid.hpp"
	"uuid.cpp"
	"uuid_impl.hpp"
)

if (WIN32)
	target_link_libraries(senc_utils PRIVATE ws2_32 Rpcrt4)
else()
	target_link_libraries(senc_utils PUBLIC libuuid)
endif()

target_link_libraries(senc_utils PUBLIC cryptopp)

set_property(TARGET senc_utils PROPERTY CXX_STANDARD 20)
