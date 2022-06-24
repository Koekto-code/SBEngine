// 24 june 2022
// SBView v0.1.0-alpha

#include "argparser.hpp"
#include <cstdint>
#include <cstring>

namespace argp
{
	// Get option with argument, i.e. "-o main.o", "-Wall"
	// sep => find only options with space between.
	char* getopt(const char* option, int argc, char** argv, bool sep)
	{
		size_t optlen = std::strlen(option);
		for (int i = 0; i < argc; ++i) {
			if (!std::memcmp(argv[i], option, optlen)) {
				size_t arglen = std::strlen(argv[i]);
				if (!sep && optlen < arglen)
					return argv[i] + optlen;
				else if (argc > i+1)
					return argv[i+1];
			}
		}
		return nullptr;
	}
	// Get option without arguments, i.e. "--help"
	bool exist(const char* option, int argc, char** argv)
	{
		size_t optlen = std::strlen(option);
		for (int i = 0; i < argc; ++i)
			if (!std::memcmp(argv[i], option, optlen))
				return true;
		
		return false;
	}
}