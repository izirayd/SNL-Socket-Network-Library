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

snl::client client;

void new_client(snl::client _client, snl::base_socket::byte_t*)
{
	client = _client;
	printf("New client: %s::%u\n", client.ip.c_str(), client.port);
}

void packet_reader(snl::client client, snl::base_socket::byte_t* buffer)
{
	printf("Packet[%s::%u]: %s\n", client.ip.c_str(), client.port, buffer);
}

void mouse_position(int x, int y, std::chrono::time_point<std::chrono::steady_clock> start)
{
	client.send("mouse_position", x, y, start);
}

int main()
{
	snl::server server;

	server["new"]  = new_client;
	server["mouse_position"] = mouse_position;

	if (server.create("79.137.80.206", 502, snl::arch_server_t::tcp_thread) == snl::status_t::success)
	{
		printf("Server success create\n");
	} else 	printf("Server error create\n");


	if (server.run(snl::type_blocked_t::non_block) == snl::status_t::success)
	{
		printf("Server success run\n");
	}
	 else 	printf("Server error run\n");


	for (;;)
		std::this_thread::sleep_for(std::chrono::seconds(1));
	

	return 0;
}