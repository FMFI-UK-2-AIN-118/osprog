#include "Reader.h"

Reader::Reader(int fd, CallbackFunc &&callback, size_t bufSize)
	: callback(std::move(callback))
{
}

void Reader::run()
{
	// just call
	//    callback(data, size);
	// when there is some data
}

void Reader::quit()
{
}
