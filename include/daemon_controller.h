#ifndef DAEMON_CONTROLLER_H__
#define DAEMON_CONTROLLER_H__

#include <daemon_common.h>

namespace daemons
	{
	namespace controller
		{

		class DaemonController
			{
			public:
				DaemonController();
				~DaemonController();

			protected:
				daemons::common::DaemonComms commChannel;

			private:
			};
		}
	}

#endif //DAEMON_CONTROLLER_H__
