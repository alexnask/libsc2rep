#include <sc2/replay.hpp>
#include <sc2/reader.hpp>
#include <sc2/dispatcher.hpp>
#include <libmpq/mpq.h>
#include <cassert>

namespace sc2 {
	Replay::Replay(std::string replaypath) {
		auto result = libmpq__archive_open(&handle, replaypath.data(), -1);

		if(result == LIBMPQ_ERROR_OPEN) {
			throw std::invalid_argument("Tried to open invalid replay (not an MPQ file or doesn't exist)");
		}

		consumeUserData(getUserHeaderData());
		consumeDetails(getFile("replay.details"));
		consumeInitData(getFile("replay.initData"));

		libmpq__archive_close(handle);
		handle = nullptr;
	}

	Replay::Replay() {
		handle = nullptr;
		frames = 0;
		build = 0;
		baseBuild = 0;
		speed = Speed::Faster;
		gameType = GameType::One;
		category = GameCategory::Ladder;
		windowsTimestamp = 0;
		region = Region::NA;
		resumed = false;
	}

	void Replay::consumeUserData(std::string data) {
		Reader reader(data);
		auto root = HeaderSchema::execute(reader);

		build = std::get<4>(std::get<1>(root));
		baseBuild = std::get<5>(std::get<1>(root));

		sc2Version = std::to_string(std::get<1>(std::get<1>(root))) + '.' + std::to_string(std::get<2>(std::get<1>(root))) + '.' + std::to_string(std::get<3>(std::get<1>(root))) + '.' + std::to_string(std::get<4>(std::get<1>(root)));
		frames = std::get<3>(root);
	}

	void Replay::consumeDetails(std::string data) {
		if(build == 0)
			throw std::logic_error("User header data must be provided before detail data.");

		// The dispatcher chooses the correct schema, runs it and correctly sets our fields :D
		Dispatcher::details(data, *this);
	}

	void Replay::consumeInitData(std::string data) {
		Reader reader(data);

		auto root = std::get<0>(InitDataSchema35032::execute(reader));

		for(const auto& player: std::get<0>(root)) {
			if(!std::get<0>(player).empty()) {
				// This is actually a player :)
				std::cout << std::get<0>(player) << std::endl << "-----------------------" << std::endl;
				if(std::get<1>(player).second)
					std::cout << "Clan tag: " << std::get<1>(player).first << std::endl;
				std::cout << "\"Observe\": " << std::get<13>(player) << std::endl;
				std::cout << "-----------------------" << std::endl << std::endl;
			}
		}

		auto gameOptions = std::get<2>(std::get<1>(root));
		lockedTeams = std::get<0>(gameOptions);
		// TODO: is this an archon mode flag?
		std::cout << "Advanced shared control: " << std::get<2>(gameOptions) << std::endl;

		std::cout << "Ranked: " << std::get<6>(gameOptions) << std::endl;
		// This must be the matchmaking flag
		std::cout << "Competitive: " << std::get<7>(gameOptions) << std::endl;
		// Is this vs AI practise?
		std::cout << "Practise: " << std::get<8>(gameOptions) << std::endl;
		std::cout << "Coop: " << std::get<9>(gameOptions) << std::endl;
		std::cout << "Observers: " << std::get<13>(gameOptions) << std::endl;

		std::cout << "Game speed: " << std::get<3>(std::get<1>(root)) << std::endl;
		std::cout << "Game type: " << std::get<4>(std::get<1>(root)) << std::endl;

		std::cout << "Max users: " << std::get<5>(std::get<1>(root)) << std::endl;
		std::cout << "Max observers: " << std::get<6>(std::get<1>(root)) << std::endl;
		std::cout << "Max players: " << std::get<7>(std::get<1>(root)) << std::endl;
	}

	std::string Replay::getUserHeaderData() {
		std::string ret;

		libmpq__off_t headerSize;
		libmpq__user_data_size(handle, &headerSize);

		assert(headerSize != 0);
		ret.resize(headerSize);

		libmpq__user_data_read(handle, &ret.front());
		return ret;
	}

	std::string Replay::getFile(std::string path) {
		// Here, we dont check to see if the handle is valid, since getFile is only called by functions called by the string constructor.
		assert(handle != nullptr);

		std::string ret;

		uint32_t fileIndex;
		auto result = libmpq__file_number(handle, path.c_str(), &fileIndex);

		if(result == LIBMPQ_ERROR_EXIST) {
			throw std::invalid_argument("Replay archive does not contain the file we are trying to unpack.");
		}

		libmpq__off_t fileSize;
		libmpq__file_size_unpacked(handle, fileIndex, &fileSize);

		ret.resize(fileSize);
		result = libmpq__file_read(handle, fileIndex, reinterpret_cast<uint8_t *>(&ret.front()), fileSize, NULL);

		if(result == LIBMPQ_ERROR_UNPACK) {
			throw std::bad_exception();
		}

		return ret;
	}
}
