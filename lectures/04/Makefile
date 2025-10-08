CXXFLAGS += -Wall -g -std=c++17

sso_size: sso_size.cpp
leak: leak.cpp

memcheck: leak
	valgrind --leak-check=yes ./leak

clean:
	rm -f sso_size leak
