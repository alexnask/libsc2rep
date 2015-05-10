#pragma once
#ifndef SC2_SERIALIZED_VERSIONED__HPP
#define SC2_SERIALIZED_VERSIONED__HPP

#include <sc2/serialized/nodes.hpp>
#include <sc2/reader.hpp>

#include <cstdint>
#include <cassert>
#include <iostream>

namespace sc2 {
	void tagcheck(Reader& reader, int expected);
	void memdump(Reader& reader);

	namespace versioned {
		template <typename T>
		class Schema {
			public:
				static Node execute(Reader& reader) {
					return T::execute(reader);
				}
		};

		template <typename T, typename... Args>
		class Struct {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 5);
					// Make our vector, get our length, assert it equal to what was given at compile time
					std::vector<Node> nodes;
					int len = reader.getVLF();

					if(len != 1 + sizeof...(Args)) {
						std::cerr << "Expected struct length " << 1 + sizeof...(Args) << ", got " << len << std::endl;
						memdump(reader);
						throw std::runtime_error("Replay file is probably corrupted");
					}

					// Push back the first node and then run our helper which will recursively reduce through the variable templates, pushing read values
					// The reference sc2protocol implementation uses this vlf to find out if this is really a member of the struct
					reader.getVLF();
					nodes.push_back(T::execute(reader));
					Struct<Args...>::helper(reader, nodes);
					return nodes;
				}

				static inline void helper(Reader& reader, std::vector<Node>& nodes) {
					reader.getVLF();
					nodes.push_back(T::execute(reader));
					Struct<Args...>::helper(reader, nodes);
				}
		};

		// Specialized helper for the last template
		template <typename T>
		class Struct<T> {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 5);

					std::vector<Node> nodes;
					int len = reader.getVLF();

					if(len != 1) {
						std::cout << "Expected struct length " << 1 << ", got " << len << std::endl;
						memdump(reader);
						throw std::runtime_error("Replay file is probably corrupted");
					}

					// The reference sc2protocol implementation uses this vlf to find out if this is really a member of the struct
					reader.getVLF();
					nodes.push_back(T::execute(reader));
					return nodes;
				}

				static inline void helper(Reader& reader, std::vector<Node>& nodes) {
					reader.getVLF();
					nodes.push_back(T::execute(reader));
				}
		};

		template <typename T>
		class Array {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 0);

					std::vector<Node> nodes;
					int len = reader.getVLF();

					for(int i = 0; i < len; i++) {
						nodes.push_back(T::execute(reader));
					}
					return nodes;
				}
		};

		template <typename T>
		class Optional {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 4);

					bool exists = reader.getByte() != 0;
					if(exists) {
						return T::execute(reader);
					}

					return Node();
				}
		};

		class BitArray {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 1);

					int len = reader.getVLF();
					return Node(reader.getAlignedBytes((len + 7) / 8));
				}
		};

		class Blob {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 2);

					int len = reader.getVLF();
					return Node(reader.getAlignedBytes(len));
				}
		};

		class Bool {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 6);

					return Node(reader.getByte() != 0);
				}
		};

		class FourCC {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 7);

					return Node(reader.getAlignedBytes(4));
				}
		};

		class Int {
			public:
				static Node execute(Reader& reader) {
					tagcheck(reader, 9);

					return Node(reader.getVLF());
				}
		};
	}
}

#endif//SC2_SERIALIZED_VERSIONED__HPP
