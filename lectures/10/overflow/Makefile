CXXFLAGS = -Wall -g

bo: CXXFLAGS += -static -fno-stack-protector
bo: bo.cpp
bo.cpp: bocpp.cpp
w: w.cpp

clean:
	rm -f bo bocpp w
