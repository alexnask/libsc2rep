#pragma once
#ifndef SC2_FORMATTING__HPP
#define SC2_FORMATTING__HPP

#include <sc2/serialized/nodes.hpp>
#include <sc2/Team.hpp>
#include <string>

namespace sc2 {
	std::string time_str(int frames);
	std::string node_str(const Node& root);
	std::string team_str(const Team& team);
	int64_t timestamp_win_to_unix(int64_t win);
	std::string timestamp_unix_str(int64_t unix, std::string fmt = "%d/%m/%Y %H:%M:%S");
}

#endif//SC2_FORMATTING__HPP
