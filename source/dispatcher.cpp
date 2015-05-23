#include <sc2/dispatcher.hpp>

namespace sc2 {
	// Split into two parts: the schema executor and the handler, because one of those two parts may be common between builds (or both)
	void Dispatcher::details(std::string data, Replay& parent) {
		if(parent.build == 35032) {
			Reader reader(data);
			auto root = Details35032::execute(reader);
			detail_handler(root, parent);
		}
	}

	template <typename RType>
	void Dispatcher::detail_handler(const RType& root, Replay& parent) {
		if(parent.build == 35032) {
			if(std::get<1>(std::get<0>(root))) {
				auto playerArr = std::get<0>(std::get<0>(root));
				int maxIndex = 0;
				for(const auto& p: playerArr) {
					Player tPlayer;
	
					tPlayer.name = std::get<0>(p);
	
					std::string raceStr = std::get<2>(p);
					if(raceStr == "Terran") {
						tPlayer.race = Player::Race::Terran;
					} else if(raceStr == "Protoss") {
						tPlayer.race = Player::Race::Protoss;
					} else if(raceStr == "Zerg") {
						tPlayer.race = Player::Race::Zerg;
					} else {
						assert(raceStr == "Random");
						tPlayer.race = Player::Race::Random;
					}
			
					auto colorArr = std::get<3>(p);
		
					tPlayer.color.a = std::get<0>(colorArr);
					tPlayer.color.r = std::get<1>(colorArr);
					tPlayer.color.g = std::get<2>(colorArr);
					tPlayer.color.b = std::get<3>(colorArr);
		
					tPlayer.teamIndex = std::get<5>(p);
					tPlayer.handicap = std::get<6>(p);
					auto outcomeInt = std::get<8>(p);
					if(outcomeInt == 1) {
						tPlayer.outcome = Player::Outcome::Win;
					} else if(outcomeInt == 2) {
						tPlayer.outcome = Player::Outcome::Loss;
					} else {
						tPlayer.outcome = Player::Outcome::Unknown;
					}
	
					maxIndex = std::max(tPlayer.teamIndex, maxIndex);

					parent.players.emplace_back(tPlayer);
				}
				parent.teams.resize(maxIndex + 1);
				for(auto& player: parent.players) {
					parent.teams[player.teamIndex].players.push_back(&player);
					parent.teams[player.teamIndex].outcome = player.outcome;
					if(player.outcome == Player::Outcome::Win) {
						parent.winner = &parent.teams[player.teamIndex];
					}
				}
			}

			parent.windowsTimestamp = std::get<5>(root);
			parent.timezone = std::get<6>(root)/36000000000;
		}
	}
}
