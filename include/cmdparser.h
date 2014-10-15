#ifndef DAEMON_CMDPARSER_H__
#define DAEMON_CMDPARSER_H__

#include <string>
#include <deque>

namespace daemons
	{
	namespace common
		{
		class cmdParser
			{
			public:
				cmdParser(int argc, char* argv[]);
				~cmdParser();

				int getCount() const
					{
					return arguments.size();
					}
				std::string getArgument(int _index) const
					{
					std::string arg;
					if (_index < getCount())
						{
						arg = arguments[_index];
						}
					return arg;
					}

			protected:
				typedef std::deque<std::string> argumentList;
				argumentList arguments;

			private:
			};
		}
	}

#endif //DAEMON_CMDPARSER_H__
