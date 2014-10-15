#ifndef DAEMON_CONTROLLER_H__
#define DAEMON_CONTROLLER_H__

#include <daemon_common.h>
#include <cmdparser.h>

namespace daemons
	{
	namespace controller
		{

		class DaemonController
			{
			public:
				DaemonController(int argc, char* argv[]);
				~DaemonController();

				bool showHelp(FILE* _output);
				int run();
			protected:
				daemons::common::DaemonComms commChannel;
				daemons::common::cmdParser arg_parser;

				void processArguments();

				bool showUserHelp;
			private:
			};
		}
	}

#endif //DAEMON_CONTROLLER_H__
