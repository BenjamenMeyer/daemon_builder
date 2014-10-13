#ifndef DAEMON_COMMON_H__
#define DAEMON_COMMON_H__

#include <string>
#include <deque>
#include <map>
#include <thread>

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
				void close();

				// Client - socket comms
				bool send_message(const std::string& _message);
				bool send_message(size_t _length, const unsigned char* _bytes);
				bool receive_message(std::string& _message);
				bool receive_message(size_t& _length, unsigned char*& _bytes);

				// Server - comms wrapped by message queues
				bool put_message(const std::string& _message);
				bool put_message(size_t _length, const unsigned char* _bytes);
				bool get_message(std::string& _message);
				bool get_message(size_t& _length, unsigned char*& _bytes);
				static void release_message(unsigned char*& _bytes);

				// server handling
				void connection_handler(int _connection_socket);
				void receive_clients(int _connection_socket);
			protected:
				bool open_channel();
				bool open_server();

				// Client - socket comms
				bool send_message(int _fd, const std::string& _message);
				bool send_message(int _fd, size_t _length, const unsigned char* _bytes);
				bool receive_message(int _fd, std::string& _message);
				bool receive_message(int _fd, size_t& _length, unsigned char*& _bytes);

				// server message information
				typedef std::deque<std::string> message_list;
				typedef std::pair<int, message_list> connection_message_pair;
				typedef std::map<int, message_list> connection_message_queue;
				connection_message_queue output_queue;
				message_list input_queue;

			private:
				bool is_controller;
				int commChannel;

				static std::string commChannelAddressPath;

				typedef std::deque<std::thread> thread_list;
				thread_list clients;
				thread_list server_thread;
				bool continue_hosting;
			};
		}
	}

#endif //DAEMON_COMMON_H__
