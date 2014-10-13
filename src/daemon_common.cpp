#include <daemon_common.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <thread>

using namespace daemons::common;

/* static */ std::string DaemonComms::commChannelAddressPath = "/run/daemon.comm";

DaemonComms::DaemonComms(bool _controller)
	{
	is_controller = _controller;
	commChannel = -1;
	continue_hosting = false;
	}
DaemonComms::~DaemonComms()
	{
	close();
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
void DaemonComms::close()
	{
	if (commChannel != -1)
		{
		::close(commChannel);
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
	
	//listen(commChannel, 5);
	return false;
	}

bool DaemonComms::send_message(const std::string& _message)
	{
	bool returnValue = false;
	if (is_controller == false)
		{
		returnValue = send_message(commChannel, _message);
		}
	return returnValue;
	}
bool DaemonComms::send_message(size_t _length, const unsigned char* _bytes)
	{
	bool returnValue = false;
	if (is_controller == false)
		{
		returnValue = send_message(commChannel, _length, _bytes);
		}
	return returnValue;
	}
bool DaemonComms::receive_message(std::string& _message)
	{
	bool returnValue = false;
	if (is_controller == false)
		{
		returnValue = receive_message(commChannel, _message);
		}
	return returnValue;
	}
bool DaemonComms::receive_message(size_t& _length, unsigned char*& _bytes)
	{
	bool returnValue = false;
	if (is_controller == false)
		{
		returnValue = receive_message(commChannel, _length, _bytes);
		}
	return returnValue;
	}

bool DaemonComms::send_message(int _fd, const std::string& _message)
	{
	return send_message(_message.length(), _message.data());
	}
bool DaemonComms::send_message(int _fd, size_t _length, const unsigned char* _bytes)
	{
	//TODO
	return false;
	}
bool DaemonComms::receive_message(int _fd, std::string& _message)
	{
	size_t length = 0;
	unsigned char* bytes = NULL;
	bool returnValue = receive_message(length, bytes);
	if (returnValue == true)
		{
		_message = std::string((const char*)bytes, length);
		}
	return returnValue;
	}
bool DaemonComms::receive_message(int _fd, size_t& _length, unsigned char*& _bytes)
	{
	bool returnValue = false;
	if (_length == 0 && _bytes == NULL)
		{
		// TODO
		}
	return returnValue;
	}

bool DaemonComms::put_message(const std::string& _message)
	{
	bool returnValue = false;
	// TODO: Add lock
	for(connection_message_queue::iterator iter = output_queue.begin();
		iter != output_queue.end();
		++iter)
		{
		iter->second.push_back(_message);
		returnValue = true;
		}
	return returnValue;
	}
bool DaemonComms::put_message(size_t _length, const unsigned char* _bytes)
	{
	std::string message((const char*)_bytes, _length);
	return put_message(message);
	}
bool DaemonComms::get_message(std::string& _message)
	{
	bool returnValue = false;
	if (input_queue.empty() == false)
		{
		_message = input_queue.front();
		input_queue.pop_front();
		returnValue = true;
		}
	return returnValue;
	}
bool DaemonComms::get_message(size_t& _length, unsigned char*& _bytes)
	{
	bool returnValue = false;
	if (_length == 0 && _bytes == NULL)
		{
		std::string message;
		returnValue = get_message(message);
		if (returnValue == true)
			{
			_bytes = (unsigned char*) calloc(sizeof(unsigned char), (message.length()+1));
			if (_bytes != NULL)
				{
				memcpy(_bytes, message.data(), message.length());
				returnValue = true;
				}
			else
				{
				returnValue = false;
				}
			}
		}
	return returnValue;
	}
/* static */ void DaemonComms::release_message(unsigned char*& _bytes)
	{
	if (_bytes != NULL)
		{
		free(_bytes);
		_bytes = NULL;
		}
	}

static void controller_handler(DaemonComms& _comm, int _connection_socket)
	{
	_comm.connection_handler(_connection_socket);
	}

void DaemonComms::connection_handler(int _connection_socket)
	{
	// TODO: Add locks
	// create a queue
	output_queue[_connection_socket] = message_list();

	std::string input_message;

	int client_commChannel = _connection_socket;
	if (client_commChannel != -1)
		{
		while (true)
			{
			// pop messages from the queue and write them to the socket
			
			for (message_list::iterator iter = output_queue[client_commChannel].begin();
				 iter != output_queue[client_commChannel].end();
				 ++iter)
				 {
				 send_message(client_commChannel, *iter);
				 }

			// read message from the socket and stick them in the queue
			if (receive_message(input_message) == true)
				{
				input_queue.push_back(input_message);
				}
		
			// TODO: Determine that the socket has gone away and exit the loop

			// so we don't eat the processor
			usleep(10);
			}

		output_queue.erase(output_queue.find(client_commChannel));
		}
	}

static void controller_server(DaemonComms& _comm, int _server_socket)
	{
	_comm.receive_clients(_server_socket);
	}
void DaemonComms::receive_clients(int _connection_socket)
	{
	int incoming = -1;
	while (continue_hosting == true)
		{
		// we don't need many client as we are simply an internal comms channel
		incoming = accept(_connection_socket, NULL, NULL);
		if (incoming != -1)
			{
			clients.push_back(std::thread(controller_handler, std::ref(*this), incoming));
			}
		}
	::close(_connection_socket);
	}
