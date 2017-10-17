#include <catch.hpp>
#include "cbuffer.h"

SCENARIO("Reference counts", "[cbuffer]") {
	GIVEN("An allocated buffer") {
		Buffer *b = buffer_new(100*1024*1024);
		THEN("refcount is 1") {
			REQUIRE(buffer_refs(b) == 1);
		}
		THEN("size is correct") {
			REQUIRE(buffer_size(b) == 100*1024*1024);
		}
		THEN("pointer to the data is non-null") {
			REQUIRE(buffer_data(b) != nullptr);
		}
		WHEN("a reference is taken") {
			Buffer *b2 = b;
			buffer_ref(b2);  // unfortunately this is how it must be used in C
			THEN("refcount on both is 2") {
				REQUIRE(buffer_refs(b) == 2);
				REQUIRE(buffer_refs(b2) == 2);
			}
			AND_THEN("both point to the same data") {
				REQUIRE(buffer_data(b) == buffer_data(b2));
			}
		}
	}
}
