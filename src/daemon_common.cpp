#include <daemon_common.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <thread>

using namespace daemons::common;

// TODO: Make this a configuration option
/* static */ std::string DaemonComms::commChannelAddressPath = "/run/daemon.comm";

static void controller_handler(DaemonComms& _comm, int _connection_socket)
	{
	_comm.connection_handler(_connection_socket);
	}
static void controller_server(DaemonComms& _comm)
	{
	_comm.receive_clients();
	}

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
	commChannel = socket(PF_UNIX, SOCK_STREAM|SOCK_NONBLOCK, 0);

	if (commChannel != -1)
		{
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
			fprintf(stderr, "Failed to connect socket. Error Code: %d", errno);
			}
		}
	else
		{
		fprintf(stderr, "Failed to open local socket. Error Code: %d", errno);
		}

	return returnValue;
	}
bool DaemonComms::open_server()
	{
	commChannel = socket(PF_UNIX, SOCK_STREAM|SOCK_NONBLOCK, 0);

	if (commChannel != -1)
		{
		unlink(commChannelAddressPath.c_str());

		struct sockaddr_un address;
		memset(&address, 0, sizeof(struct sockaddr_un));

		address.sun_family = AF_UNIX;
		snprintf(address.sun_path, sizeof(address.sun_path),
		         "%s", commChannelAddressPath.c_str());
		int result = bind(commChannel,
		                  (const struct sockaddr*) &address,
						  sizeof(struct sockaddr_un));
		if (result == 0)
			{
			result = listen(commChannel, 5);

			if (result == 0)
				{
				server_thread = std::thread(controller_server, std::ref(*this));
				}
			else
				{
				fprintf(stderr, "Failed to listen on socket. Error Code: %d", errno);
				}
			}
		else
			{
			fprintf(stderr, "Failed to bind socket. Error Code: %d", errno);
			}
		}
	else
		{
		fprintf(stderr, "Failed to open local socket. Error Code: %d", errno);
		}
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
	bool returnValue = false;
	if (_length > 0 && _bytes != NULL)
		{
		DaemonComms::msg_header header;
		memset(&header, 0, sizeof(DaemonComms::msg_header));
		header.length = calculate_message_length(_length);
		header.version = 1;
		header.message_type = 1;
		header.data_length = _length;

		size_t header_size = calculate_message_length(0);
		ssize_t sent_data = send(_fd, &header, header_size, 0);
		if (sent_data > 0)
			{
			sent_data = send(_fd, _bytes, _length, 0);
			if (sent_data > 0)
				{
				returnValue = (size_t(sent_data) == _length);
				}
			}
		}
	return returnValue;
	}
uint8_t DaemonComms::receive_message(int _fd, std::string& _message)
	{
	size_t length = 0;
	unsigned char* bytes = NULL;
	uint8_t returnValue = receive_message(length, bytes);
	if (returnValue == 0)
		{
		_message = std::string((const char*)bytes, length);
		}
	return returnValue;
	}
uint8_t DaemonComms::receive_message(int _fd, size_t& _length, unsigned char*& _bytes)
	{
	uint8_t returnValue = -1;
	if (_length == 0 && _bytes == NULL)
		{
		size_t header_size = calculate_message_length(0);
		DaemonComms::msg_header header;
		while(true)
			{
			memset(&header, 0, sizeof(DaemonComms::msg_header));
			
			size_t incoming_data = recv(_fd, &header, header_size, MSG_PEEK);

			if (incoming_data == header_size)
				{
				unsigned char recvd_data[header.length+1];
				size_t msg_length = header.length + 1;

				while (true)
					{
					memset(recvd_data, 0, msg_length);
					
					incoming_data = recv(_fd, recvd_data, header.length, MSG_WAITALL);
					if (incoming_data == header.length)
						{
						// send the data back up
						unsigned char* return_buffer = (unsigned char*) calloc(sizeof(unsigned char), msg_length);
						if (return_buffer != NULL)
							{
							// copy it all, including the extra termination byte we added on the end
							memcpy(return_buffer, recvd_data, msg_length);

							// give it to the caller, and only report the actual message size
							_bytes = recvd_data;
							_length = header.length;

							// the sole success case
							returnValue = 0;
							}
						else
							{
							// error allocating buffer
							returnValue = -8;
							}
						break;
						}
					else if (incoming_data == 0)
						{
						// orderly shutdown by other side - exit the loop
						returnValue = -7;
						break;
						}
					else if (incoming_data == -1)
						{
						switch(errno)
							{
							case EAGAIN: // aka EWOULDBLOCK
								// repeat
								continue;
							default:
								// error
								returnValue = -6;
								break;
							};
						}
					else
						{
						// we didn't receive all the data
						returnValue = -5;
						break;
						}
					}
				break;
				}
			else if (incoming_data == 0)
				{
				// orderly shutdown by other side or no data to recv - exit the loop
				returnValue = -4;
				break;
				}
			else if (incoming_data == -1)
				{
				switch(errno)
					{
					case EAGAIN: // aka EWOULDBLOCK 
						// repeat
						continue;
					default:
						returnValue = -3;
						// error
						break;
					};
				}
			else
				{
				// we didn't receive all the data
				returnValue = -2;
				break;
				}
			}
		}
	return returnValue;
	}

bool DaemonComms::put_message(const std::string& _message)
	{
	bool returnValue = false;
	std::lock_guard<std::mutex> lock(message_mutex);
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

void DaemonComms::connection_handler(int _connection_socket)
	{
	// create a queue
	output_queue[_connection_socket] = message_list();

	std::string input_message;

	if (_connection_socket != -1)
		{
		while (true)
			{
			std::lock_guard<std::mutex> lock(message_mutex);
			// pop messages from the queue and write them to the socket
			
			for (message_list::iterator iter = output_queue[_connection_socket].begin();
				 iter != output_queue[_connection_socket].end();
				 ++iter)
				 {
				 send_message(_connection_socket, *iter);
				 }

			// read message from the socket and stick them in the queue
			int recv_result = receive_message(_connection_socket, input_message);
			if (recv_result == 0)
				{
				input_queue.push_back(input_message);
				}

			// make sure the socket is still good
			if (check_socket(_connection_socket) == true)
				{
				break;
				}

			// so we don't eat the processor, only check for messages once every 100ms
			usleep(100);
			}

		output_queue.erase(output_queue.find(_connection_socket));
		::close(_connection_socket);
		}
	}
bool DaemonComms::check_socket(int _fd)
	{
	bool returnSocketInvalid = false;

	// Determine that the socket has gone away and exit the loop
	// try a peek
	size_t header_size = calculate_message_length(0);
	DaemonComms::msg_header header;
	memset(&header, 0, sizeof(DaemonComms::msg_header));

	int peek_result = recv(_fd, &header, header_size, MSG_PEEK);
	if (peek_result == -1)
		{
		switch(errno)
			{
			case EBADF:
			case ECONNREFUSED:
			case ENOTCONN:
				returnSocketInvalid = true;
				break;
			default:
				break;
			};
		}
	return returnSocketInvalid;
	}

void DaemonComms::receive_clients()
	{
	int incoming = -1;
	while (continue_hosting == true)
		{
		// we don't need many client as we are simply an internal comms channel
		incoming = accept(commChannel, NULL, NULL);
		if (incoming != -1)
			{
			clients.push_back(std::thread(controller_handler, std::ref(*this), incoming));
			}
		}
	::close(commChannel);
	}
