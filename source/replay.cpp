#include <sc2/replay.hpp>
#include <sc2/reader.hpp>
#include <sc2/dispatcher.hpp>
#include <libmpq/mpq.h>
#include <cassert>

#include <sc2/serialized/bitpack.hpp>
typedef sc2::bitpack::Schema<sc2::bitpack::Struct<sc2::bitpack::Struct<sc2::bitpack::Array<0, 5, sc2::bitpack::Struct<sc2::bitpack::Blob<0, 8>, sc2::bitpack::Optional<sc2::bitpack::Blob<0, 8>>, sc2::bitpack::Optional<sc2::bitpack::Blob<40, 0>>, sc2::bitpack::Optional<sc2::bitpack::Int<0, 8>>, sc2::bitpack::Optional<sc2::bitpack::Int<0, 32>>, sc2::bitpack::Int<0, 32>, sc2::bitpack::Struct<sc2::bitpack::Optional<sc2::bitpack::Int<0, 8>>>, sc2::bitpack::Struct<sc2::bitpack::Optional<sc2::bitpack::Int<0, 8>>>, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Int<0, 32>, sc2::bitpack::Int<0, 2>, sc2::bitpack::Blob<0, 9>, sc2::bitpack::Blob<0, 9>, sc2::bitpack::Blob<0, 9>, sc2::bitpack::Blob<0, 7>>>, sc2::bitpack::Struct<sc2::bitpack::Int<0, 32>, sc2::bitpack::Blob<0, 10>, sc2::bitpack::Struct<sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Int<0, 2>, sc2::bitpack::Int<0, 2>, sc2::bitpack::Int<0, 2>, sc2::bitpack::Int<0, 64>>, sc2::bitpack::Int<0, 3>, sc2::bitpack::Int<0, 3>, sc2::bitpack::Int<0, 5>, sc2::bitpack::Int<0, 5>, sc2::bitpack::Int<0, 5>, sc2::bitpack::Int<1, 4>, sc2::bitpack::Int<0, 6>, sc2::bitpack::Int<1, 8>, sc2::bitpack::Int<0, 8>, sc2::bitpack::Int<0, 8>, sc2::bitpack::Int<0, 8>, sc2::bitpack::Int<0, 32>, sc2::bitpack::Blob<0, 11>, sc2::bitpack::Blob<0, 8>, sc2::bitpack::Int<0, 32>, sc2::bitpack::Array<0, 5, sc2::bitpack::Struct<sc2::bitpack::BitArray<0, 6>, sc2::bitpack::BitArray<0, 8>, sc2::bitpack::BitArray<0, 6>, sc2::bitpack::BitArray<0, 8>, sc2::bitpack::BitArray<0, 2>, sc2::bitpack::BitArray<0, 7>>>, sc2::bitpack::Int<0, 6>, sc2::bitpack::Int<0, 7>, sc2::bitpack::Array<0, 6, sc2::bitpack::Blob<40, 0>>, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool, sc2::bitpack::Bool>, sc2::bitpack::Struct<sc2::bitpack::Int<0, 3>, sc2::bitpack::Int<0, 5>, sc2::bitpack::Int<0, 5>, sc2::bitpack::Array<0, 5, sc2::bitpack::Struct<sc2::bitpack::Int<0, 8>, sc2::bitpack::Optional<sc2::bitpack::Int<0, 4>>, sc2::bitpack::Int<0, 4>, sc2::bitpack::Struct<sc2::bitpack::Optional<sc2::bitpack::Int<0, 5>>>, sc2::bitpack::Struct<sc2::bitpack::Optional<sc2::bitpack::Int<0, 8>>>, sc2::bitpack::Int<0, 6>, sc2::bitpack::Int<0, 7>, sc2::bitpack::Int<0, 7>, sc2::bitpack::Int<0, 2>, sc2::bitpack::Int<0, 32>, sc2::bitpack::Blob<0, 9>, sc2::bitpack::Blob<0, 9>, sc2::bitpack::Blob<0, 9>, sc2::bitpack::Array<0, 4, sc2::bitpack::Blob<0, 9>>, sc2::bitpack::Optional<sc2::bitpack::Int<0, 8>>, sc2::bitpack::Array<0, 17, sc2::bitpack::Int<0, 32>>, sc2::bitpack::Blob<0, 7>, sc2::bitpack::Array<0, 9, sc2::bitpack::Int<0, 32>>, sc2::bitpack::Optional<sc2::bitpack::Int<0, 4>>, sc2::bitpack::Blob<0, 9>>>, sc2::bitpack::Int<0, 32>, sc2::bitpack::Optional<sc2::bitpack::Int<0, 4>>, sc2::bitpack::Bool, sc2::bitpack::Int<0, 32>, sc2::bitpack::Int<0, 6>, sc2::bitpack::Int<0, 7>>>>> InitDataSchema;
#include <sc2/formatting.hpp>
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

		build = root.arr[1].arr[4].num;
		baseBuild = root.arr[1].arr[5].num;

		sc2Version = std::to_string(root.arr[1].arr[1].num) + '.' + std::to_string(root.arr[1].arr[2].num) + '.' + std::to_string(root.arr[1].arr[3].num) + '.' + std::to_string(root.arr[1].arr[4].num);
		frames = root.arr[3].num;
	}

	void Replay::consumeDetails(std::string data) {
		if(build == 0)
			throw std::logic_error("User header data must be provided before detail data.");

		// The dispatcher chooses the correct schema, runs it and correctly sets our fields :D
		Dispatcher::details(data, *this);
	}

	void Replay::consumeInitData(std::string data) {
		Reader reader(data);

		auto root = InitDataSchema::execute(reader);
		//std::cout << data << std::endl;
		std::cout << node_str(root) << std::endl;
		/*
		int playerNum = reader.getByte();
		for(int i = 0; i < playerNum; i++) {
			// We don't do anythng with player names at this point
			reader.getPascalString();
			reader.skip(5);
		}

		// Todo: document those somewhere
		reader.skip(322);

		// This is the SC2 account string
		reader.getPascalString();

		reader.skip(891);

		std::string magic = reader.getAlignedBytes(4);
		assert(magic == "s2ma");

		reader.skip(2);
		std::string regionStr = reader.getAlignedBytes(2);

		// TODO: verify and add cases, although all replays will be Unknown in beta anyway :D
		if(regionStr == "NA") {
			region = Region::NA;
		} else if(regionStr == "EU") {
			region = Region::EU;
		} else if(regionStr == "KR") {
			region = Region::KR;
		} else {
			region = Region::Unknown;
		}
		*/
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
