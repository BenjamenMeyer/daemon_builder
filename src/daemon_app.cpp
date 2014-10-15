#include <daemon_app.h>

#include <stdio.h>

using namespace daemon::application;

daemonApp::daemonApp(int argc, char* argv[]) : commChannel(false), arg_parser(argc, argv)
	{
	daemonize = false;
	showUserHelp = false;

	processArguments();
	if (showUserHelp == false)
		{
		commChannel.open();
		}
	}
daemonApp::~daemonApp()
	{
	commChannel.close();
	}

void daemonApp::processArguments()
	{
	programName = arg_parser.getArgument(0);
	std::string arg;
	for (int i = 1; i < arg_parser.getCount(); ++i)
		{
		arg = arg_parser.getArgument(1);
		if (arg == "--daemonize" ||
		    arg == "-d")
			{
			daemonize = true;
			}
		else if (arg == "--standalone" ||
			     arg == "-s"	)
			{
			daemonize = false;
			}
		else if (arg == "--help")
			{
			showUserHelp = true;
			}
		else if (arg == "--config" ||
				 arg == "-c")
			{
			++i;
			configFile = arg_parser.getArgument(i);
			if (configFile.empty() == true ||
			    configFile.length() == 0)
				{
				showUserHelp = true;
				}
			}
		}
	}
bool daemonApp::showHelp(FILE* _output)
	{
	if (showUserHelp == true)
		{
		fprintf(_output, "%s <options>\n", programName.c_str());
		fprintf(_output, "\n\t--daemonize (-d)\tDaemonize the application");
		fprintf(_output, "\n\t--standalone (-s)\tKeep application in foreground");
		fprintf(_output, "\n\t--config (-c) <file>\tUse the specified configuration file");
		fprintf(_output, "\n\t--help (-h)\tShow application help\n");
		}
	return showUserHelp;
	}

int daemonApp::run()
	{
	return 0;
	}

int main(int argc, char* argv[])
	{
	daemonApp theDaemon(argc, argv);
	if (theDaemon.showHelp(stderr) == true)
		{
		return 0;
		}
	else
		{
		return theDaemon.run();
		}
	}
