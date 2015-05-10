#include <sc2/player.hpp>
#include <cassert>

namespace sc2 {
	Player::Player(Node root, int build) {
		if(build == 35032) {
			name = root.arr[0].str;
	
			auto raceStr = root.arr[2].str;
			if(raceStr == "Terran") {
				race = Race::Terran;
			} else if(raceStr == "Protoss") {
				race = Race::Protoss;
			} else if(raceStr == "Zerg") {
				race = Race::Zerg;
			} else {
				assert(raceStr == "Random");
				race = Race::Random;
			}
	
			auto colorArr = root.arr[3].arr;
			color.a = colorArr[0].num;
			color.r = colorArr[1].num;
			color.g = colorArr[2].num;
			color.b = colorArr[3].num;
	
			teamIndex = root.arr[5].num;
			handicap = root.arr[6].num;
			auto outcomeInt = root.arr[8].num;
			if(outcomeInt == 1) {
				outcome = Outcome::Win;
			} else if(outcomeInt == 2) {
				outcome = Outcome::Loss;
			} else {
				outcome = Outcome::Unknown;
			}
		}
	}
}
