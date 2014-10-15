#include <daemon_controller.h>

using namespace daemons::controller;

DaemonController::DaemonController(int argc, char* argv[]) : commChannel(true), arg_parser(argc, argv)
	{
	showUserHelp = false;
	commChannel.open();
	processArguments();
	}
DaemonController::~DaemonController()
	{
	}
bool DaemonController::showHelp(FILE* _output)
	{
	return false;
	}

void DaemonController::processArguments()
	{
	}
int DaemonController::run()
	{
	return 0;
	}

int main(int argc, char* argv[])
	{
	DaemonController controller(argc, argv);
	if (controller.showHelp(stderr) == true)
		{
		return 0;
		}
	else
		{
		return controller.run();
		}
	}
