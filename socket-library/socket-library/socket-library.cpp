#pragma once

#if defined _WIN32
#include "../../socket.hpp"
#else
#include "socket.hpp"
#endif

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

void read(snl::client client, snl::base_socket::byte_t *buffer)
{
	printf("Packet: %s %s\n", buffer, client.ip.c_str());
}

void new_connect(snl::client client, snl::base_socket::byte_t *buffer)
{
	printf("New client: %s::%u\n", client.ip.c_str(), client.port);
}

uint32_t max_queue = 0;

namespace snl {

	struct thread_t
	{
		thread_t() = default;
 		thread_t(const uint32_t &id) { index = id; }
		uint32_t        index = 0;
		std::thread::id device_index;
	};

	template <typename T>
	struct atomic_wrapper
	{
		std::atomic<T>     atomic_obj;
		atomic_wrapper() : atomic_obj() {}
		atomic_wrapper(const std::atomic<T> &a)	:atomic_obj(a.load())	{}
		atomic_wrapper(const atomic_wrapper &other):atomic_obj(other.atomic_obj.load()){}
		atomic_wrapper &operator=(const atomic_wrapper &other)	{ atomic_obj.store(other.atomic_obj.load());	}
		atomic_wrapper &operator=(const T &other)	{ atomic_obj = other;	}
	};

	struct packet_t
	{
		atomic_wrapper<bool> is_use { false };
	
		bool is_write = false;
		packet_t() { }
		std::size_t index_queue = 0;
		snl::base_socket::byte_t *packet_buffer = nullptr;
		uint32_t size_buffer = 0;
		thread_t *for_thread = nullptr;
	
	
		void executable(thread_t thread, uint32_t index_queue)
		{
			if (index_queue > max_queue)
				max_queue = index_queue;

			//printf("executable packet thread: %u count_queue: %u max: %u\n", thread.index, index_queue, max_queue);
			//printf("%u\r", max_queue);

			uint64_t number_packet;
			memcpy(&number_packet, packet_buffer, 8);

			printf("number_packet: %lld thread:  %u\n", number_packet, thread.index);

		}

		// not-multithread
		bool create_buffer(const uint32_t &size_buf) {

			if (packet_buffer != nullptr || size_buf == 0)
				return false;

			size_buffer = size_buf;
			packet_buffer = new snl::base_socket::byte_t[size_buffer];

			return true;
		}

		// not-multithread
		bool delete_buffer()
		{
			if (packet_buffer == nullptr)
				return false;

			size_buffer = 0;
			delete[] packet_buffer;
			return true;
		}
	};

	class executable_queue_t
	{
	public:
		std::vector<snl::packet_t> queue_packet;
		std::size_t last_queue = 0;


		// not-multithread
		bool create_queue(const std::size_t &size_queue, const uint32_t &base_size_packet)
		{
			queue_packet.resize(size_queue);

			std::size_t index_queue = 0;
			for (auto &packet : queue_packet) 
			{
				packet.index_queue = index_queue;
				index_queue++;
				packet.create_buffer(base_size_packet);
			}
			
			return true;
		}
	};

	class pool_thread_t
	{
	      public:

			  std::vector<thread_t> pool_thread;
			  std::chrono::microseconds executable_sleep = std::chrono::microseconds(100);
			  uint32_t count_thread = 0;
			  uint32_t last_pool    = 0;

			  executable_queue_t executable_queue;

			  // not-multithread
			  void create_thread(const uint32_t &count_th)
			  {
				  count_thread = count_th;

				  pool_thread.resize(count_thread);

				  for (uint32_t i = 0; i < count_thread; i++)
				  {
					  pool_thread[i].index = i;
					  std::thread obj_thread(&pool_thread_t::executable, this, pool_thread[i]);
					  pool_thread[i].device_index = obj_thread.get_id();
					  obj_thread.detach();
				  }
			  }

			  // multithread
			  void executable(thread_t thread)
			  {
				  printf("thread start[%u]\n", thread.index);
				
				  while (true)
				  {
					 // for (auto &packet : executable_queue.queue_packet) 
					  for (uint32_t index_queue = 0; index_queue < executable_queue.queue_packet.size(); index_queue++)
					  {
						 snl::packet_t *packet = &executable_queue.queue_packet[index_queue];

						 if (packet)
						  if (packet->for_thread != nullptr)
							  if (packet->for_thread->index == thread.index)
								  if (packet->is_use.atomic_obj.load() && packet->is_write)
								  {
									  packet->executable(*packet->for_thread, index_queue);
									  packet->is_write = false;   // is_write должен быть строго вызван раньше atomic_obj
									  packet->is_use.atomic_obj = false;
								  }

						  index_queue++;
					  }
						  		 
					
					  std::this_thread::sleep_for(executable_sleep);
				  }
			  }

			  // multithread
			  packet_t* begin()
			  {
				  uint32_t use_packet_index = 0;
				  bool is_find_packet = false; // найден ли пакет?

				  for (size_t i = 0; i < executable_queue.queue_packet.size(); i++)
				  {
					  if (!executable_queue.queue_packet[i].is_use.atomic_obj.load()) {
						  executable_queue.queue_packet[i].is_use.atomic_obj = true;
						  use_packet_index = i;
						  is_find_packet  = true;
						  break;
					  }
				  }

				  if (is_find_packet) {
					  packet_t *packet_descriptor = &executable_queue.queue_packet[use_packet_index];
					  packet_descriptor->is_write = false;
					  return packet_descriptor;
				  }

				  if (!is_find_packet)
				  {
					  printf("No packet buffer!\n");
				  }

				  return nullptr;
			  }

			  // multithread
			  void end(packet_t *packet_descriptor)
			  {
				  if (packet_descriptor == nullptr)
					  return;

				  if (last_pool == count_thread)
					  last_pool = 0;

				  packet_descriptor->for_thread = &pool_thread[last_pool];
				
				  // Пакет записан и может быть обработан пулом потоков
				  packet_descriptor->is_write = true;

				  last_pool++;
			  }
	};
}


using namespace std::chrono_literals;



void mouse_position(int x, int y, std::chrono::time_point<std::chrono::steady_clock> start)
{
	std::chrono::time_point<std::chrono::steady_clock> end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	printf("Mouse position: %d, %d [%lf ms]\n", x, y, elapsed.count());
}

int main()
{
	snl::client   client;

	client["mouse_position"] = mouse_position;
	snl::status_t status;

	if ((status = client.connect("79.137.80.206", 502, snl::arch_server_t::tcp_thread)) == snl::status_t::success)
	{
		 printf("Clinet success connect\n");
	}
	else 	printf("Clinet error connect: %d\n", (int) status);


	client.run(snl::type_blocked_t::non_block);

	POINT pos, last_pos;
	last_pos.x = 0;
	last_pos.y = 0;

	for (;;)
	{
		if (GetCursorPos(&pos))
		{
			if (pos.x != last_pos.x || pos.y != last_pos.y) {

				std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::high_resolution_clock::now();

				client.send("mouse_position", pos.x, pos.y, start);

				last_pos.x = pos.x;
				last_pos.y = pos.y;
			}
		}
	
		//std::this_thread::sleep_for(std::chrono::microseconds(800));
	}

/*	snl::pool_thread_t pool_thread;
	pool_thread.create_thread(8);
	pool_thread.executable_queue.create_queue(50000, 64);
 
	uint64_t number_packet = 0;

	for (size_t i = 0; i < 50; i++)
	{
		snl::packet_t *packet_descriptor = pool_thread.begin();

		memcpy(packet_descriptor->packet_buffer, (void *)&number_packet, 8);
		number_packet++;
		pool_thread.end(packet_descriptor);
	}

	for (;;)
	{
		snl::packet_t *packet_descriptor = pool_thread.begin();
		
		memcpy(packet_descriptor->packet_buffer, (void *) &number_packet, 8);
		number_packet++;
		pool_thread.end(packet_descriptor);

		std::this_thread::sleep_for(std::chrono::microseconds(1000));

		//std::this_thread::sleep_for(std::chrono::seconds(1));
	}



	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::microseconds(10000));
	}*/

	return 0;
}