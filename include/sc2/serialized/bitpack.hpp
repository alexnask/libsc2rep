#pragma once
#ifndef SC2_SERIALIZED_BITPACK__HPP
#define SC2_SERIALIZED_BITPACK__HPP

#include <sc2/serialized/nodes.hpp>
#include <sc2/reader.hpp>

#include <cstdint>
#include <cassert>
#include <iostream>

namespace sc2 {
	namespace bitpack {
		template <typename T>
		class Schema {
			public:
				static Node execute(Reader& reader) {
					return T::execute(reader);
				}
		};

		template <int Lb, int Ub>
		class Int {
			public:
				static Node execute(Reader& reader) {
					assert(Ub <= 64);
					return Node(Lb + (int64_t)reader.getBits(Ub));
				}
		};

		template <typename T, typename... Args>
		class Struct {
			public:
				static Node execute(Reader& reader) {
					std::vector<Node> nodes;

					nodes.push_back(T::execute(reader));
					Struct<Args...>::helper(reader, nodes);
					return nodes;
				}

				static inline void helper(Reader& reader, std::vector<Node>& nodes) {
					nodes.push_back(T::execute(reader));
					Struct<Args...>::helper(reader, nodes);
				}
		};

		// Specialized helper for the last template
		template <typename T>
		class Struct<T> {
			public:
				static Node execute(Reader& reader) {
					std::vector<Node> nodes;

					nodes.push_back(T::execute(reader));
					return nodes;
				}

				static inline void helper(Reader& reader, std::vector<Node>& nodes) {
					nodes.push_back(T::execute(reader));
				}
		};

		template <int Lb, int Ub, typename T>
		class Array {
			public:
				static Node execute(Reader& reader) {
					std::vector<Node> nodes;
					int len = Int<Lb, Ub>::execute(reader).num;
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
					bool exists = reader.getBits(1) != 0;
					if(exists) {
						return T::execute(reader);
					}

					return Node();
				}
		};

		template <int Lb, int Ub>
		class BitArray {
			public:
				static Node execute(Reader& reader) {
					int len = Int<Lb, Ub>::execute(reader).num;
					// Apparently BitArrays can get quite big, let's just make them read bytes into a string :)
					assert(len > 0);

					std::string ret = reader.getBytes(len / 8);
					ret += (char)reader.getBits(len % 8);
					std::cout << "BitArr: " << ret << " (len: " << len << " bits)" << std::endl;
					return Node(ret);
				}
		};

		template <int Lb, int Ub>
		class Blob {
			public:
				static Node execute(Reader& reader) {
					int len = Int<Lb, Ub>::execute(reader).num;
					auto str = reader.getAlignedBytes(len);
					if(len > 0) std::cout << "Blobstr: " << str << "(len: " << len << ")" << std::endl;
					return Node(str);
				}
		};

		class Bool {
			public:
				static Node execute(Reader& reader) {
					return Node(reader.getBits(1) != 0);
				}
		};


		template <int Tag, typename T>
		class Pair {
			public:
				static inline bool check(int t) {
					return t == Tag;
				}

				static inline void execute(Reader& reader) {
					T::execute(reader);
				}
		};

		// Only Pairs should be passed as parameters
		template <int Lb, int Ub, typename P, typename... Ps>
		class Choice {
			public:
				static Node execute(Reader& reader) {
					int tag = Int<Lb, Ub>::execute(reader).num;
					return helper(tag, reader);
				}

				static inline Node helper(int tag, Reader& reader) {
					if(P::check(tag)) return P::execute(reader);
					return Choice<Lb, Ub, Ps...>::helper(tag, reader);
				}
		};

		template <int Lb, int Ub, typename P>
		class Choice<Lb, Ub, P> {
			public:
				static Node execute(Reader& reader) {
					int tag = Int<Lb, Ub>::execute(reader).num;
					return helper(tag, reader);
				}

				static inline Node helper(int tag, Reader& reader) {
					if(P::check(tag)) return P::execute(reader);
					throw std::runtime_error("Replay file is probably corrupted");
				}
		};

		class FourCC {
			public:
				static Node execute(Reader& reader) {
					return Node(reader.getBytes(4));
				}
		};
	}
}

#endif//SC2_SERIALIZED_BITPACK__HPP
