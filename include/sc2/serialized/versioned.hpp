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
				static inline auto execute(Reader& reader) -> decltype(T::execute(reader)) {
					return T::execute(reader);
				}
		};

		template <typename T, typename... Args>
		class Struct {
			public:
				static inline auto execute(Reader& reader) -> std::tuple<decltype(T::execute(reader)), decltype(Args::execute(reader))...> {
					tagcheck(reader, 5);
					// Make our vector, get our length, assert it equal to what was given at compile time
					std::tuple<decltype(T::execute(reader)), decltype(Args::execute(reader))...> nodes;
					int len = reader.getVLF();

					if(len != 1 + sizeof...(Args)) {
						std::cerr << "Expected struct length " << 1 + sizeof...(Args) << ", got " << len << std::endl;
						memdump(reader);
						throw std::runtime_error("Replay file is probably corrupted");
					}

					// Push back the first node and then run our helper which will recursively reduce through the variable templates, pushing read values
					// The reference sc2protocol implementation uses this vlf to find out if this is really a member of the struct
					reader.getVLF();

					std::get<0>(nodes) = T::execute(reader);

					Struct<Args...>::template helper<1>(reader, nodes);
					return nodes;
				}

				template <int Index, typename Tuple>
				static inline void helper(Reader& reader, Tuple& nodes) {
					reader.getVLF();
					std::get<Index>(nodes) = T::execute(reader);
					Struct<Args...>::template helper<Index + 1>(reader, nodes);
				}
		};

		// Specialized helper for the last template
		template <typename T>
		class Struct<T> {
			public:
				static inline auto execute(Reader& reader) -> std::tuple<decltype(T::execute(reader))> {
					tagcheck(reader, 5);

					std::tuple<decltype(T::execute(reader))> nodes;
					int len = reader.getVLF();

					if(len != 1) {
						std::cout << "Expected struct length 1, got " << len << std::endl;
						memdump(reader);
						throw std::runtime_error("Replay file is probably corrupted");
					}

					// The reference sc2protocol implementation uses this vlf to find out if this is really a member of the struct
					reader.getVLF();
					std::get<0>(nodes) = T::execute(reader);
					return nodes;
				}

				template <int Index, typename Tuple>
				static inline void helper(Reader& reader, Tuple& nodes) {
					reader.getVLF();
					std::get<Index>(nodes) = T::execute(reader);
				}
		};

		template <typename T>
		class Array {
			public:
				static inline auto execute(Reader& reader) -> std::vector<decltype(T::execute(reader))> {
					tagcheck(reader, 0);

					std::vector<decltype(T::execute(reader))> nodes;
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
				static inline auto execute(Reader& reader) -> std::pair<decltype(T::execute(reader)), bool> {
					tagcheck(reader, 4);

					bool exists = reader.getByte() != 0;
					if(exists) {
						return std::make_pair(T::execute(reader), true);
					}

					std::pair<decltype(T::execute(reader)), bool> res;
					std::get<1>(res) = false;

					return res;
				}
		};

		class BitArray {
			public:
				static inline std::string execute(Reader& reader) {
					tagcheck(reader, 1);

					int len = reader.getVLF();
					return reader.getAlignedBytes((len + 7) / 8);
				}
		};

		class Blob {
			public:
				static inline std::string execute(Reader& reader) {
					tagcheck(reader, 2);

					int len = reader.getVLF();
					return reader.getAlignedBytes(len);
				}
		};

		class Bool {
			public:
				static inline bool execute(Reader& reader) {
					tagcheck(reader, 6);

					return reader.getByte() != 0;
				}
		};

		class FourCC {
			public:
				static inline std::string execute(Reader& reader) {
					tagcheck(reader, 7);

					return reader.getAlignedBytes(4);
				}
		};

		class Int {
			public:
				static inline int64_t execute(Reader& reader) {
					tagcheck(reader, 9);

					return reader.getVLF();
				}
		};
	}
}

#endif//SC2_SERIALIZED_VERSIONED__HPP
