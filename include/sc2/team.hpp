#pragma once
#ifndef SC2_TEAM__HPP
#define SC2_TEAM__HPP
#include <memory>
#include <sc2/player.hpp>

namespace sc2 {
	struct Team {
		std::vector<Player *> players = {};
		Player::Outcome outcome = Player::Outcome::Unknown;
	};
}

#endif//SC2_TEAM__HPP
