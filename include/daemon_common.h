#ifndef DAEMON_CONTROLLER_H__
#define DAEMON_CONTROLLER_H__

#include <string>

namespace daemons
	{
	namespace common
		{
		class DaemonComms
			{
			public:
				DaemonComms(bool _controller);
				~DaemonComms();

				bool open();
			protected:
				
				bool open_channel();
				bool open_server();
			private:
				bool is_controller;
				int commChannel;

				static std::string commChannelAddressPath;
			};
		}
	}

#endif //DAEMON_CONTROLLER_H__
