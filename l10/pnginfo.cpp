/**
 * A simplistic program to print out information about a PNG file.
 * Prints out dimensions and any tEXt sections.
 *
 * Compiled with either
 *     g++ -std=c++11 -static -fno-stack-protector -g -o pnginfo  pnginfo.cpp
 * or with
 *     g++ -m32 -std=c++11 -static -fno-stack-protector -g -o pnginfo32  pnginfo.cpp
 * for the 32 bit version.
 */
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

#include <fstream>
#include <vector>
#include <stdexcept>
#include <memory>

struct ChunkHdr {
	uint32_t length;
	char type[4];
};

struct IhdrChunk {
	uint32_t width;
	uint32_t height;
	uint8_t bitDepth;
	uint8_t colorType;
	uint8_t compression;
	uint8_t filter;
	uint8_t interlace;
};

struct TextChunk {
	char keyword[80]; // according to spec, keywords are 1 - 79 chars
	char *text; // texts can have variable size
};

class PngInfo {
public:
	std::vector<TextChunk> textChunks;
	IhdrChunk hdr;

	PngInfo(const char *fname)
	{
		std::ifstream file(fname);
		if (!file)
			throw std::runtime_error("Can't open file");

		const unsigned char pngHdr[8] = { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A };
		char fileHdr[8] = {0,};

		if (!file.read(fileHdr, sizeof(fileHdr)))
			throw std::runtime_error("Can't read PNG header");

		if (strncmp(fileHdr, (const char*)pngHdr, 8))
				throw std::runtime_error("PNG header does not match");

		ChunkHdr chunkHdr;
		while (file.read(reinterpret_cast<char*>(&chunkHdr), sizeof(chunkHdr))) {
			chunkHdr.length = ntohl(chunkHdr.length); // numbers in PNG are little endian

			if (!strncmp("IHDR", chunkHdr.type, 4)) {
				loadIHDR(file, chunkHdr);
			}
			else if (!strncmp("tEXt", chunkHdr.type, 4)) {
				loadTEXT(file, chunkHdr);
			}
			else {
				if (!file.ignore(chunkHdr.length))
					throw std::runtime_error("Can't skip chunk");
			}

			if (!file.ignore(4))
				throw std::runtime_error("Can't read chunk checksum");
			}
	}

	~PngInfo()
	{
		for (const auto &tc : textChunks)
			free(tc.text); // aloccated with strndup
	}

	void loadIHDR(std::ifstream &file, const ChunkHdr &chunkHdr)
	{
		static_assert(sizeof(IhdrChunk) >= 13, "Ehm, should not happen ;)");
		if (chunkHdr.length != 13)
			throw std::runtime_error("Bad IHDR chunk size");
		if (!file.read(reinterpret_cast<char*>(&hdr), 13))
			throw std::runtime_error("Can't read IHDR");

		hdr.width = ntohl(hdr.width);
		hdr.height = ntohl(hdr.height);
	}

	void loadTEXT(std::ifstream &file, const ChunkHdr &chunkHdr)
	{
		std::unique_ptr<char[]> data{new char[chunkHdr.length]};

		if (!file.read(data.get(), chunkHdr.length))
			throw std::runtime_error("Can't read tEXt chunk");

		parseTEXT(data.get(), chunkHdr);
	}

	void parseTEXT(const char *data, const ChunkHdr &chunkHdr)
	{
		TextChunk tc;
		size_t keywordLen = strlen(data);

		tc.text = strndup(data + keywordLen + 1, chunkHdr.length - keywordLen - 1);
		memcpy(tc.keyword, data, chunkHdr.length);

		textChunks.push_back(tc);
	}
};

int main(int argc, char* argv[])
{
	if (argc < 2) {
		return 1;
	}

	PngInfo png(argv[1]);
	printf("%s: %dx%d\n", argv[1], png.hdr.width, png.hdr.height);
	for (const auto &tc : png.textChunks)
		printf("%s: %s\n", tc.keyword, tc.text);
}


extern "C" {
// Write the messages to stderr, which is always line buffered.
// (Though for good measure we also explicitly flush it ;)
// Stdout is line buffered only on interactive consoles, so if the output
// is piped and the program crashes (as expected after "expliting" this function)
// it would not be flushed / printed.
//
// Note: there are badWolf_<your_ais_login> functions in the commited binary
// for each student.
void badWolf(void) { fprintf(stderr, "Well done!\n\n"); fflush(stderr); }
void badWolf_siska2(void) { fprintf(stderr, "Well done siska2!\n\n"); fflush(stderr); }
}
