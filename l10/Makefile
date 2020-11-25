CXXFLAGS = -std=c++17 -g -static -fno-stack-protector

test:
	@./test.sh

pnginfo: pnginfo.cpp

bo: bo.cpp

USERS ?= missing_users_file
functions:
	awk '{print "void badWolf_" $$1 "(void) { fprintf(stderr, \"Well done " $$1 "!\\n\\n\"); fflush(stderr); }"}' $(USERS)

.PHONY: test functions
