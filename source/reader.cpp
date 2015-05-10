#include <sc2/reader.hpp>
#include <cstdint>
#include <cassert>
#include <iostream>
namespace sc2 {
	Reader::Endianness Reader::endianness = Reader::Endianness::Little;

	Reader::Reader(std::string d) : data(std::move(d)) {
		static const union {
        	uint32_t i;
        	char c[4];
    	} bint = { 0x01020304 };

		Reader::endianness = bint.c[0] == 1 ? Endianness::Big : Endianness::Little;
	}

	void Reader::align() {
		bitpos = 0;
	}

	std::string Reader::getAlignedBytes(int len) {
		align();
		bytepos += len;
		return data.substr(bytepos - len, len);
	}

	std::string Reader::getBytes(int len) {
		std::string ret;
		ret.resize(len);
		for(int i = 0; i < len; i++) {
			ret += (char)getByte();
		}
		return ret;
	}

	int8_t Reader::getByte() {
		if(bitpos == 0) {
			return data[bytepos++];
		}

		// This is what is left of our byte appended with zero bits
		int8_t start = data[bytepos] << bitpos;

		// Go to next byte. The bit position stays the same because we are reading 1 byte
		bytepos++;
		int8_t rest = data[bytepos] >> (8 - bitpos);

		return start | rest;
	}

	int64_t Reader::getBits(int bits) {
		assert(bits <= 64);

		if(bits == 8) return getByte();

		if(bits < 8) {
			int64_t current = (int8_t)(data[bytepos] << bitpos);
			// We have enough bits in the current byte!
			if(bits < 8 - bitpos) {
				// We must shift right 'bitpos' bits to get to the original byte with the first bits truncated, then (8 - bitpos - bits) to truncate the end bytes we need
				current >>= (8 - bits);
				return current;
			} else {
				// We need to get the next byte
				bytepos++;
				int8_t next = data[bytepos];
				// Set it's bit position
				bitpos = bits - (8 - bitpos);
				// Keep only the bits we will need
				next >>= 8 - bitpos;
				// Then we shift our current value to align it correctly before ORing it
				current >>= 8 - bits;
				return current | next;
			}
		}

		// If we have more than 8 bits, we will compose calls
		int64_t composed = 0;
		while(bits > 8) {
			int8_t byte = getByte();
			// Shift the previous byte left, OR in the new byte
			composed = (composed << 8) | byte;

			bits -= 8;
		}

		if(bits > 0) {
			// OR in the few remaining bits
			composed = (composed << bits) | (int8_t)getBits(bits);
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
