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

	void Dispatcher::detail_handler(Node root, Replay& parent) {
		if(parent.build == 35032) {
			auto playerArr = root.arr[0];
			int maxIndex = 0;
			for(const auto& p: playerArr.arr) {
				parent.players.emplace_back(p, parent.build);

				maxIndex = std::max(parent.players.back().teamIndex, maxIndex);
			}
			parent.teams.resize(maxIndex + 1);
			for(auto& player: parent.players) {
				parent.teams[player.teamIndex].players.push_back(&player);
				parent.teams[player.teamIndex].outcome = player.outcome;
				if(player.outcome == Player::Outcome::Win) {
					parent.winner = &parent.teams[player.teamIndex];
				}
			}

			parent.windowsTimestamp = root.arr[5].num;
			parent.timezone = root.arr[6].num/36000000000;
		}
	}
}
