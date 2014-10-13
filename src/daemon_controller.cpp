#include <daemon_controller.h>

using namespace daemons::controller;

DaemonController::DaemonController() : commChannel(true)
	{

	}
DaemonController::~DaemonController()
	{
	}

int main(int argc, char* argv[])
	{

	DaemonController controller;
	return 1;
	}
