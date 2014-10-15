#include <cmdparser.h>

#include <string>

using namespace daemons::common;

cmdParser::cmdParser(int argc, char* argv[])
	{
	for (int i=0; i < argc; ++i)
		{
		std::string arg = argv[i];
		arguments.push_back(arg);
		}
	}
cmdParser::~cmdParser()
	{
	}

