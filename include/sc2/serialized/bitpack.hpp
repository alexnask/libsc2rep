#pragma once
#ifndef SC2_SERIALIZED_BITPACK__HPP
#define SC2_SERIALIZED_BITPACK__HPP

#include <sc2/serialized/nodes.hpp>
#include <sc2/reader.hpp>

#include <cstdint>
#include <cassert>

namespace sc2 {
	namespace bitpack {
		template <typename T>
		class Schema {
			public:
				static inline auto execute(Reader& reader) -> decltype(T::execute(reader)) {
					return T::execute(reader);
				}
		};

		template <int Lb, int Ub>
		class Int {
			public:
				static int execute(Reader& reader) {
					assert(Ub <= 64);
					return Lb + (int64_t)reader.getBits(Ub);
				}
		};

		template <typename T, typename... Args>
		class Struct {
			public:
				static inline auto execute(Reader& reader) -> std::tuple<decltype(T::execute(reader)), decltype(Args::execute(reader))...> {
					using tuple_type = std::tuple<decltype(T::execute(reader)), decltype(Args::execute(reader))...>;
					tuple_type nodes;

					std::get<0>(nodes) = T::execute(reader);
					Struct<Args...>::template helper<1>(reader, nodes);
					return nodes;
				}

				template <int Index, typename Tuple>
				static inline void helper(Reader& reader, Tuple& nodes) {
					std::get<Index>(nodes) = T::execute(reader);
					Struct<Args...>::template helper<Index + 1>(reader, nodes);
				}
		};

		// Specialized helper for the last template
		template <typename T>
		class Struct<T> {
			public:
				static inline auto execute(Reader& reader) -> std::tuple<decltype(T::execute(reader))> {
					std::tuple<decltype(T::execute(reader))> nodes;

					std::get<0>(nodes) = T::execute(reader);
					return nodes;
				}

				template <int Index, typename Tuple>
				static inline void helper(Reader& reader, Tuple& nodes) {
					std::get<Index>(nodes) = T::execute(reader);
				}
		};

		template <int Lb, int Ub, typename T>
		class Array {
			public:
				static inline auto execute(Reader& reader) -> std::vector<decltype(T::execute(reader))> {
					std::vector<decltype(T::execute(reader))> nodes;

					int len = Int<Lb, Ub>::execute(reader);
					for(int i = 0; i < len; i++) {
						nodes.push_back(T::execute(reader));
					}

					return nodes;
				}
		};

		template <typename T>
		class Optional {
			public:
				static inline auto execute(Reader& reader) -> std::pair<decltype(T::execute(reader)), bool> {
					bool exists = reader.getBits(1) != 0;
					if(exists) {
						return std::make_pair(T::execute(reader), true);
					}

					std::pair<decltype(T::execute(reader)), bool> pair;
					std::get<1>(pair) = false;
					return pair;
				}
		};

		template <int Lb, int Ub>
		class BitArray {
			public:
				static inline std::string execute(Reader& reader) {
					int len = Int<Lb, Ub>::execute(reader);
					// Apparently BitArrays can get quite big, let's just make them read bytes into a string :)
					assert(len >= 0);

					std::string ret = reader.getBytes(len / 8);
					if (len % 8) {
						ret += (char)reader.getBits(len % 8);
					}
					return ret;
				}
		};

		template <int Lb, int Ub>
		class Blob {
			public:
				static inline std::string execute(Reader& reader) {
					int len = Int<Lb, Ub>::execute(reader);
					return reader.getAlignedBytes(len);
				}
		};

		class Bool {
			public:
				static inline bool execute(Reader& reader) {
					return reader.getBits(1) != 0;
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
				static inline Node execute(Reader& reader) {
					int tag = Int<Lb, Ub>::execute(reader);
					return helper(tag, reader);
				}

				static inline Node helper(int tag, Reader& reader) {
					if(P::check(tag)) return Node(P::execute(reader));
					return Choice<Lb, Ub, Ps...>::helper(tag, reader);
				}
		};

		template <int Lb, int Ub, typename P>
		class Choice<Lb, Ub, P> {
			public:
				static inline Node execute(Reader& reader) {
					int tag = Int<Lb, Ub>::execute(reader);
					return helper(tag, reader);
				}

				static inline Node helper(int tag, Reader& reader) {
					if(P::check(tag)) return Node(P::execute(reader));
					throw std::runtime_error("Replay file is probably corrupted");
				}
		};

		class FourCC {
			public:
				static inline std::string execute(Reader& reader) {
					return reader.getBytes(4);
				}
		};
	}
}

#endif//SC2_SERIALIZED_BITPACK__HPP
