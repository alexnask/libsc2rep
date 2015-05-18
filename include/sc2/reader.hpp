#pragma once
#ifndef SC2_READER__HPP
#define SC2_READER__HPP

#include <sc2/serialized/nodes.hpp>

#include <string>
#include <sstream>
#include <algorithm>
#include <memory>
#include <cstdint>

namespace sc2 {
	class Reader {
		public:
			enum class Endianness {
				Little, Big
			};

			Reader(const std::string& d);

			// Aligns the bit position to the byte position (sets bitpos = 0)
			void align();
			void alignForward();

			// Aligns and gets some bytes, convenience function
			std::string getAlignedBytes(int len);

			std::string getBytes(int len);
			uint64_t getBits(int bits);

			uint8_t getByte();

			// Note, those do not reset alignment, call align() if you need to
			void skip(int bytes);
			void rewind(int bytes);
			int8_t peek();

			int64_t getVLF();
		private:

			std::string data;
			static Endianness endianness;

			// Position of the current byte in our data.
			int bytepos = 0;
			// Position of the current bit relative to the current byte.
			int bitpos = 0;
	};
}

#endif//SC2_READER__HPP
