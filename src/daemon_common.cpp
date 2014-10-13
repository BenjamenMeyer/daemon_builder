#include <daemon_common.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

using namespace daemons::common;

/* static */ std::string DaemonComms::commChannelAddressPath = "/run/daemon.comm";

DaemonComms::DaemonComms(bool _controller)
	{
	commChannel = -1;
	}
DaemonComms::~DaemonComms()
	{
	}
	
bool DaemonComms::open()
	{
	if (is_controller)
		{
		return open_channel();
		}
	else
		{
		return open_server();
		}
	}

bool DaemonComms::open_channel()
	{
	bool returnValue = false;
	commChannel = socket(PF_UNIX, SOCK_STREAM, 0);

	struct sockaddr_un address;
	memset(&address, 0, sizeof(struct sockaddr_un));
	address.sun_family = AF_UNIX;
	snprintf(address.sun_path, sizeof(address.sun_path),
			 "%s", commChannelAddressPath.c_str());

	int result = connect(commChannel,
	                     (const struct sockaddr*) &address,
						 sizeof(struct sockaddr_un));
	if (result == 0)
		{
		returnValue = true;
		}
	else
		{
		fprintf(stderr, "Failed to open local socket. Error Code: %d", errno);
		}

	return returnValue;
	}
bool DaemonComms::open_server()
	{
	return false;
	}
