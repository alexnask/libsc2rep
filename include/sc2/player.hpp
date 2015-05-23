#pragma once
#ifndef SC2_PLAYER__HPP
#define SC2_PLAYER__HPP

#include <sc2/serialized/nodes.hpp>
#include <string>

namespace sc2 {
	struct Team;

	class Player {
		public:
			struct Color {
				int a;
				int r;
				int g;
				int b;
			};

			enum class Race {
				Terran, Zerg, Protoss, Random
			};

			enum class Outcome {
				Win, Loss, Unknown
			};

			std::string name;
			Color color;
			Race race;
			int teamIndex;
			// This is a raw pointer because all players are owned by team so they will be destroyed at the same time
			Team *team;
			int handicap;
			Outcome outcome;
	};
}

#endif//SC2_PLAYER__HPP
