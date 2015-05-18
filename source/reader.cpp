#include <sc2/reader.hpp>
#include <cstdint>
#include <cassert>
#include <iostream>
#include <bitset>
namespace sc2 {
	Reader::Endianness Reader::endianness = Reader::Endianness::Little;

	Reader::Reader(const std::string& d) : data(std::move(d)) {
		static const union {
        	uint32_t i;
        	char c[4];
    	} bint = { 0x01020304 };

		Reader::endianness = bint.c[0] == 1 ? Endianness::Big : Endianness::Little;
	}

	void Reader::align() {
		if(bitpos != 0) {
			bitpos = 0;
			bytepos++;
		}
	}

	std::string Reader::getAlignedBytes(int len) {
		align();
		bytepos += len;
		return data.substr(bytepos - len, len);
	}

	std::string Reader::getBytes(int len) {
		std::string ret;
		ret.resize(len);
		for(decltype(len) i = 0; i < len; i++) {
			ret += (char)getByte();
		}
		return ret;
	}

	int8_t Reader::getByte() {
		if(bitpos == 0) {
			return data[bytepos++];
		}

		uint8_t start = ((uint8_t)data[bytepos] >> bitpos) << bitpos;
		bytepos++;
		uint8_t rest = ((uint8_t)data[bytepos] << (uint8_t)(8 - bitpos)) >> (uint8_t)(8 - bitpos);
		return start | rest;
	}

	uint64_t Reader::getBits(int bits) {
		assert(bits <= 64);

		if(bits == 0) return 0;
		if(bits == 8) return getByte();

		if(bits < 8) {
			uint8_t current = (uint8_t)(data[bytepos]) >> bitpos;

			if(bits == 8 - bitpos) {
				bitpos = 0;
				bytepos++;
				return current;
			} else if(bits < 8 - bitpos) {
				// We have enough bits in the current byte!
				current = (uint8_t)(current << (8 - bits)) >> (8 - bits);
				bitpos += bits;
				return current;
			} else {
				bytepos++;
				uint8_t next = data[bytepos];
				bitpos = bits - (8 - bitpos);
				next <<= (8 - bitpos);
				current <<= bitpos;
				return current | next;
			}
		}

		// If we have more than 8 bits, we will compose calls
		uint64_t composed = 0;
		while(bits > 8) {
			uint8_t byte = getByte();
			// Shift the previous byte left, OR in the new byte
			composed = (composed << 8) | (uint8_t)byte;

			bits -= 8;
		}

		if(bits > 0) {
			// OR in the few remaining bits
			composed = (composed << bits) | (uint8_t)getBits(bits);
		}

		return composed;
	}

	int64_t Reader::getVLF() {
		int64_t res;
		int byte = (uint8_t)getByte();

		bool negative = (byte & 1) != 0;
		res = (byte & 0x7F) >> 1;
		int bits = 6;
		while(byte & 0x80) {
			byte = (uint8_t)getByte();
			res |= (int64_t)(byte & 0x7F) << bits;
			bits += 7;
		}
		assert(bits <= 64);
		return negative ? -res : res;
	}

	void Reader::skip(int bytes) {
		bytepos += bytes;
	}

	void Reader::rewind(int bytes) {
		bytepos -= bytes;
	}

	int8_t Reader::peek() {
		auto ret = getByte();
		bytepos -= 1;
		return ret;
	}
}
