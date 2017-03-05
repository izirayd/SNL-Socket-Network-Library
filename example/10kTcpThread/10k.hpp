/*
    For this example use x64 parametr build, because x32 have max ~1500 thread.
*/

#pragma once

#include "../../Socket.hpp"
#include <iostream>

int32_t CountClient = 0;

void NewClient(std::socket_t Socket, std::base_socket::byte_t *Buffer)
{
	CountClient++;
	printf("\rCount client: %d", CountClient);
}

void EndClient(std::socket_t Socket, std::base_socket::byte_t *Buffer)
{
	CountClient--;
	printf("\rCount client: %d", CountClient);
}

int main()
{
	std::base_socket::init_socket();

	std::server server;

	server["new"] = NewClient;
	server["end"] = EndClient;
	
	server.Create("127.0.0.1", 500, std::arch_server_t::tcp_thread);
	server.Run(std::type_blocked_t::non_block);

	std::client client;

	for (int32_t i = 0; i < 10000; i++)	{
	  client.Connect("127.0.0.1", 500, std::arch_server_t::tcp_thread);
	  client.Run(std::type_blocked_t::non_block);
	}

	for (;;)	
		std::this_thread::sleep_for(1s);
	
	return 0;
}

