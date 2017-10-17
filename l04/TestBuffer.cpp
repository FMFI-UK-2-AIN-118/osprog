#include <catch.hpp>

#include "Buffer.h"
#include <cstring>

#define BigStr "Hello"
#define SmallStr "Hi!"
#define Big (10*1024*1024)
#define Small 100

TEST_CASE("Referece counts", "[buffer]")
{
	Buffer buf(Big);

	REQUIRE(buf.refs() == 1);
	REQUIRE(buf.size() == Big);

	strcpy(buf.data(), BigStr);
	REQUIRE(!strcmp(BigStr, buf.cdata()));

	{
		INFO("Copy constructed b1");
		Buffer b1(buf);
		REQUIRE(buf.refs() == 2);
		REQUIRE(b1.refs() == 2);
		REQUIRE(buf.cdata() == b1.cdata());

		{
			Buffer other(100);
			Buffer b2(other);
			strcpy(b2.data(), SmallStr);

			REQUIRE(other.refs() == 2);
			REQUIRE(b2.refs() == 2);

			INFO("operator=");
			b2 = b1;
			REQUIRE(b1.refs() == 3);
			REQUIRE(b2.refs() == 3);
			REQUIRE(!strcmp(BigStr, buf.cdata()));
			REQUIRE(!strcmp(BigStr, b1.cdata()));
			REQUIRE(!strcmp(BigStr, b2.cdata()));

			REQUIRE(other.refs() == 1);
			REQUIRE(!strcmp(SmallStr, other.cdata()));
		}
		INFO("other and b2 are out of scope");
		REQUIRE(buf.refs() == 2);
		REQUIRE(b1.refs() == 2);
	}
	INFO("b1 is out of scope");
	REQUIRE(buf.refs() == 1);
}

using Catch::Matchers::Equals;

SCENARIO("Contents", "[Buffer]") {
	GIVEN("An empty buffer") {
		Buffer b;
		THEN("size is 0") {
			REQUIRE(b.size() == 0);
		}

		WHEN("data is appended using the 'c string' convention") {
			b.append("Hello");
			THEN("contents, capacity and size is correct") {
				REQUIRE(b.size() == 5);
				REQUIRE(b.capacity() >= 5);
				REQUIRE(!strncmp("Hello", b.cdata(), 5));
			}
			AND_WHEN("more data of a given size is appended") {
				b.append(" xxx", 1); // `xxx` should be ignored
				THEN("contents, capacity and size is correct") {
					REQUIRE(b.size() == 6);
					REQUIRE(b.capacity() >=6);
					REQUIRE(!strncmp("Hello ", b.cdata(), 6));
				}
			}
			AND_WHEN("remove is called") {
				b.remove(1,3);
				THEN("the contents/size is correct") {
					REQUIRE(b.size() == 2);
					REQUIRE(!strncmp("Ho", b.cdata(), 2));
				}
			}
		}
	}
	GIVEN("A pre-allocated buffer") {
		Buffer b(10);
		WHEN("data are modified through data()") {
			// 10 is the size of the buffer, the actual string is 9 chars with the terminating zero
			strncpy(b.data(), "abcdefgh", 10);
			THEN("they can be read through cdata()") {
				REQUIRE(!strncmp("abcdefgh", b.cdata(), 9)); // we don't know what the last byte contains
			}
		}
	}
}

TEST_CASE("Contents (chained)", "[Buffer]")
{
	Buffer b;
	REQUIRE(b.size() == 0);

	b.append("Hello");
	REQUIRE(b.size() == 5);
	REQUIRE(b.capacity() >= 5);

	b.append(" ", 1);
	REQUIRE(b.size() == 6);
	REQUIRE(b.capacity() >=6);

	b.append("world!");
	REQUIRE(b.size() == 12);
	REQUIRE(b.capacity() >=12);
	REQUIRE(!strncmp("Hello world!", b.cdata(), 12));

	b.remove(2, 3);
	REQUIRE(b.size() == 9);
	REQUIRE(!strncmp("He world!", b.cdata(), 9));

	b.chop(3);
	REQUIRE(b.size() == 6);
	REQUIRE(!strncmp("He wor", b.cdata(), 6));
}

SCENARIO("Capacity", "[Buffer]") {
	GIVEN("An empty buffer") {
		Buffer b;
		THEN("It's size is 0") {
			REQUIRE(b.size() == 0);
		}
		WHEN("space is reserved") {
			b.reserve(1024*1024);
			THEN("size is still 0") {
				REQUIRE(b.size() == 0);
			}
			THEN("capacity is at least the requested ammount") {
				REQUIRE(b.capacity() >= 1024*1024);
			}
			AND_WHEN("data is added") {
				b.append("Hi");
				THEN("size is correct") {
					REQUIRE(b.size() == 2);
				}
				THEN("data is correct") {
					REQUIRE(!strncmp("Hi", b.cdata(), 2));
				}
				THEN("capacity is still reserved") {
					REQUIRE(b.capacity() >= 1024*1024);
				}

				AND_WHEN("the buffer is squeezed") {
					b.squeeze();
					THEN("memory is release") {
						REQUIRE(b.capacity() < 1024*1024);
					}
					THEN("data is still intact") {
						REQUIRE(b.size() == 2);
						REQUIRE(!strncmp("Hi", b.cdata(), 2));
					}
				}
				AND_WHEN("reserve is called with something small") {
					b.reserve(1);
					THEN("data is intact / of correct size") {
						REQUIRE(b.size() == 2);
						REQUIRE(b.capacity() >= 2);
						REQUIRE(!strncmp("Hi", b.cdata(), 2));
					}
				}
				AND_WHEN("reserve is called with something small") {
					b.reserve(0);
					THEN("data is intact / of correct size") {
						REQUIRE(b.size() == 2);
						REQUIRE(b.capacity() >= 2);
						REQUIRE(!strncmp("Hi", b.cdata(), 2));
					}
				}
			}
		}
	}
}

TEST_CASE("Capacity (chained operations)", "[Buffer]")
{
	Buffer b;
	REQUIRE(b.size() == 0);

	b.reserve(1024*1024);
	REQUIRE(b.size() == 0);
	REQUIRE(b.capacity() >= 1024*1024);

	b.append("Hi");
	REQUIRE(b.size() == 2);
	REQUIRE(b.capacity() >= 1024*1024);

	b.squeeze();
	REQUIRE(b.size() == 2);
	REQUIRE(b.capacity() < 1024*1024);
	REQUIRE(!strncmp("Hi", b.cdata(), 2));

	b.reserve(1);
	REQUIRE(b.size() == 2);
	REQUIRE(b.capacity() >= 2);

	b.reserve(0);
	REQUIRE(b.size() == 2);
	REQUIRE(b.capacity() >= 2);
}

SCENARIO("Reallocations", "[Buffer]") {
	GIVEN("A buffer") {
		Buffer b;

		std::size_t lastCap = b.capacity();
		int reallocs = 0;

		WHEN("Adding a lot of small pieces") {
			for(int i = 0; i < 1123123; ++i) {
				b.append("Hello");
				if (b.capacity() != lastCap) {
					lastCap = b.capacity();
					++reallocs;
				}
			}
			THEN("there are only a couple of reallocations and data/size are correct") {
				REQUIRE(b.size() == 5*1123123);
				REQUIRE(reallocs < 123); // well should certainly be <20
				REQUIRE(!strncmp(b.cdata(), "HelloHello", 10));
				REQUIRE(!strncmp(b.cdata() + b.size() - 10, "HelloHello", 10));
				// some more random "security checks" ;)
				REQUIRE(!strncmp(b.cdata() + 1230, "HelloHello", 10));
				REQUIRE(!strncmp(b.cdata() + 7770, "HelloHello", 10));
				REQUIRE(!strncmp(b.cdata() + 55000, "HelloHello", 10));
				REQUIRE(!strncmp(b.cdata() + 550000, "HelloHello", 10));
				REQUIRE(!strncmp(b.cdata() + 5500000, "HelloHello", 10));
			}
			AND_WHEN("it is resized back to something small") {
				b.resize(10);
				THEN("memory is release") {
					REQUIRE(b.capacity() < 1024*1024);
				}
				THEN("data shuld still be correct") {
					REQUIRE(!strncmp(b.cdata(), "HelloHello", 10));
				}
			}
			AND_WHEN("it is slowly resized back via chop") {
				reallocs = 0;
				lastCap = b.capacity();
				while(b.size() > 10) {
					b.chop(10);
					if (b.capacity() != lastCap) {
						lastCap = b.capacity();
						++reallocs;
					}
				}
				THEN("there are only a couple (or none) reallocations") {
					REQUIRE(b.size() <= 10);
					REQUIRE(reallocs < 123);
				}
			}
		}
	}
}
