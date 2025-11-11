add_library (senc_utils
	"concepts.hpp"
	"math.hpp"
	"Exception.hpp"
	"Exception.cpp"
	"Socket.hpp"
	"Socket.cpp"
	"Socket_impl.hpp"
	"poly.hpp"
	"poly_impl.hpp"
	"Random.hpp"
	"Random_impl.hpp"
	"ModInt.hpp"
	"ModInt_impl.hpp"
	"ModInt.cpp"
	"Fraction.hpp"
	"Fraction_impl.hpp"
	"Fraction.cpp"
	"Group.hpp"
)

target_link_libraries(senc_utils PRIVATE ws2_32 cryptopp::cryptopp)

set_property(TARGET senc_utils PROPERTY CXX_STANDARD 20)
