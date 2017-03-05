#pragma once
#include "../../Socket.hpp"

using namespace std::chrono_literals;
std::chrono::time_point<std::chrono::steady_clock> start;
std::chrono::time_point<std::chrono::steady_clock> end;

void ReadPacket(std::socket_t Socket, std::base_socket::byte_t *Buffer)
{
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	printf("\nPacket[%d - %lf ms]: %s", Socket, elapsed.count(), Buffer);	
}

int main()
{
	std::base_socket::init_socket();

	std::server server;
	std::client client;

	server["read"] = ReadPacket;

	server.Create("127.0.0.1", 500, std::arch_server_t::udp_thread);
	server.Run(std::type_blocked_t::non_block);

	client.Connect("127.0.0.1", 500, std::arch_server_t::udp_thread);

	for (;;) {
		start = std::chrono::high_resolution_clock::now();
		client.Send("Hello, World!\0", 15);

		std::this_thread::sleep_for(1ms);
	}

	for (;;)
		std::this_thread::sleep_for(1ms);
	
	return 0;
}