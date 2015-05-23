#pragma once
#ifndef SC2_REPLAY__HPP
#define SC2_REPLAY__HPP
#include <sc2/team.hpp>

#include <string>
#include <vector>
#include <memory>

struct mpq_archive;

namespace sc2 {
	class Replay {
		public:
			enum class Speed {
				Slower, Slow, Normal, Fast, Faster
			};

			enum class GameCategory {
				Ladder, Private
			};

			enum class GameType {
				One, Two, Three, FFA
			};

			enum class Region {
				NA, EU, SEA, KR, Unknown
			};

			Replay(std::string replaypath);
			Replay();

			// Consumes archive header user data
			void consumeUserData(std::string data);
			// Consumes replay.initData data [region]
			void consumeInitData(std::string data);
			// Consumes replay.details data []
			void consumeDetails(std::string data);

			// Those members are public by design, a replay is meant to be a collection of data, no special actions we can do here and I dislike getters/setters for the sake of it

			// Total frames (16 per second) the game lasted
			int frames;

			// SC2 client build number
			int build = 0;

			// SC2 engine build number
			int baseBuild = 0;

			// Game speed (Slower, Slow, Normal, Fast, Faster)
			Speed speed;

			// Game type at creation (1v1, 2v2, 3v3, 4v4, FFA)
			GameType gameType;

			// Game category (Ladder, Private)
			GameCategory category;

			// Pointer to a Map object :D
			//std::unique_ptr<Map> map = nullptr;

			// Win timestamp of the UTC time the game ended at as recorded by the client
			int64_t windowsTimestamp;

			// Timezone!
			int timezone;

			// Region the game was played on
			Region region;

			// Game events
			//std::vector<Event> events = {};

			// Teams, that hold players
			std::vector<Team> teams = {};

			// Observers in the game
			//std::vector<Observer> observers = {};

			// Game messages, sorted by ascending timestamp
			//std::vector<Message> messages = {};

			// Pings received by people in the game
			// std::vector<Ping> pings = {};

			// Vector of players!
			std::vector<Player> players = {};

			// Player that recorded the replay (those are raw because we don't have ownership, the vectors do but we guarantee their lifetime are the same as the Replay's)
			Player *recorder = nullptr;

			// Pointer to the winning team or nullptr if none is specified/valid
			Team *winner = nullptr;

			// Whether this game was resumed from replay
			bool resumed;

			// Whether the teams of this game are locked
			bool lockedTeams;

			// Representation of the sc2 version
			std::string sc2Version = "";
		private:
			// libmpq archive of the replay file
			mpq_archive *handle;

			// Get a file's contents from the MPQ archive
			std::string getFile(std::string path);

			// Get the archive's user header data
			std::string getUserHeaderData();
	};
}

#endif//SC2_REPLAY__HPP
