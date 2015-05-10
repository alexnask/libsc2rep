#include <sc2/formatting.hpp>
#include <sstream>
#include <ctime>

namespace sc2 {
	std::string time_str(int frames) {
		std::string ret;

		std::stringstream builder;

		int seconds = frames / 16;
		int hours = seconds / 3600;
		seconds -= hours * 3600;
		int minutes = seconds / 60;
		seconds -= minutes * 60;

		builder << hours << ":" << minutes << ":" << seconds;
		ret = builder.str();

		return ret;
	}

	std::string node_str(const Node& root) {
		std::string ret;
		switch(root.type) {
			case Node::Type::ByteString: {
				ret = "\"" + root.str + "\"";
			break;
			}
			case Node::Type::Int: {
				ret = std::to_string(root.num);
			break;
			}
			case Node::Type::None: {
				ret = "None";
			break;
			}
			case Node::Type::Array: {
				ret = "[";
				bool first = true;
				for(auto& node: root.arr) {
					if(first) {
						first = false;
					} else {
						ret += ",";
					}
					ret += " " + node_str(node);
				}
				ret += " ]";
			break;
			}
			case Node::Type::Bool: {
				ret = root.b ? "true" : "false";
			break;
			}
			default:
			break;
		}
		return ret;
	}

	std::string team_str(const Team& team) {
		std::string ret;
		bool first = true;
		for(const auto& p: team.players) {
			if(first) {
				first = false;
			} else {
				ret += ", ";
			}
			ret += p->name;
		}

		return ret;
	}

	int64_t timestamp_win_to_unix(int64_t win) {
		static int64_t magic = 11644473600;
		return (win / 10000000 - magic);
	}

	std::string timestamp_unix_str(int64_t unix, std::string fmt) {
		std::string ret;
		ret.resize(fmt.size() + 4);

	    const time_t rawtime = (const time_t)unix;
	    struct tm *dt;	
	    dt = localtime(&rawtime);

	    strftime(&ret.front(), ret.size(), fmt.data(), dt);

	    return ret;
	}
}
