#ifndef DAEMON_COMM_H__
#define DAEMON_COMM_H__

#include <string>
#include <deque>
#include <map>
#include <thread>
#include <atomic>
#include <mutex>

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
				void receive_clients();
			protected:
				bool open_channel();
				bool open_server();

				// Client - socket comms
				bool send_message(int _fd, const std::string& _message);
				bool send_message(int _fd, size_t _length, const unsigned char* _bytes);

				// return values:
				//		 0 - success
				//		-1 - parameter error
				// 		-2 - did not receive the entire header
				// 		-3 - other error mid-header
				// 		-4 - otherside shutdown mid-header
				// 		-5 - did not receive the entire message after receiving the header
				// 		-6 - other error mid-message after header sent
				// 		-7 - otherside shutdown mid-message after header sent
				// 		-8 - buffer allocation error
				uint8_t receive_message(int _fd, std::string& _message);
				uint8_t receive_message(int _fd, size_t& _length, unsigned char*& _bytes);
				bool check_socket(int _fd);

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
				std::thread server_thread;
				std::atomic_bool continue_hosting;

				std::mutex message_mutex;

				struct msg_header
					{
					uint32_t length;		// length of entire message including header
					uint32_t version;		// version of message system
					uint32_t message_type;	// message type
					uint32_t data_length;	// length of just the message data
					uint8_t message[0];		// message data
					};

				inline size_t calculate_message_length(size_t _data_length)
					{
					return ((sizeof(struct msg_header) - sizeof(uint8_t)) + (sizeof(uint8_t)*_data_length));
					};
			};
		}
	}

#endif //DAEMON_COMM_H__
