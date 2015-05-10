#pragma once
#ifndef SC2_SERIALIZED_NODE__HPP
#define SC2_SERIALIZED_NODE__HPP

#include <cstdint>
#include <string>
#include <vector>
#include <memory>

namespace sc2 {
	// Any serialized data (documented in the s2protocol github repository)
	class Node {
		public:
			enum class Type {
				ByteString,
				Array,
				Int, // All integers stored here
				Bool,
				None
			};

			Node(std::string data);
			Node(std::vector<Node> data);
			Node(int64_t data);
			Node(bool data);
			Node();

			Node(const Node& other);
			Node& operator=(const Node& other);
			~Node();

			Type type;

			union {
				std::string str;
				std::vector<Node> arr;
				int64_t num;
				bool b;
			};
	};
}

#endif//SC2_SERIALIZED_NODE__HPP
