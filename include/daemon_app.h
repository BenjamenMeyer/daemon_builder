#ifndef DAEMON_APP_H__
#define DAEMON_APP_H__

#include <daemon_comm.h>
#include <cmdparser.h>

namespace daemons
	{
	namespace application
		{

		class daemonApp
			{
			public:
				daemonApp(int argc, char* argv[]);
				~daemonApp();

				bool showHelp(FILE* _output);

				int run();
			protected:
				daemons::common::DaemonComms commChannel;
				daemons::common::cmdParser arg_parser;

				void processArguments();

				std::string programName;
				std::string configFile;
				bool daemonize;
				bool showUserHelp;

			private:
			};

		}
	}

#endif //DAEMON_APP_H__
