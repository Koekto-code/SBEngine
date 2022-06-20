// 20 june 2022
// SB Engine view program v0.4.0-alpha

#ifndef ARGPARSER_HPP
#define ARGPARSER_HPP

namespace argp
{
	// Get option with argument, i.e. "-o main.o", "-Wall"
	// sep => find only options with space between.
	char* getopt(const char* option, int argc, char** argv, bool sep = false);
	// Get option without arguments, i.e. "--help"
	bool exist(const char* option, int argc, char** argv);
}

#endif