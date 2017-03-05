#pragma once

#include "../../Socket.hpp"
#include <iostream>


using namespace std::chrono_literals;
std::chrono::time_point<std::chrono::steady_clock> start;
std::chrono::time_point<std::chrono::steady_clock> end;

void ReadPacket(std::socket_t Socket, std::base_socket::byte_t *Buffer)
{
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	printf("\nPacket[%d - %lf ms]: %s", Socket, elapsed.count(), Buffer);	
}

//
//void ReadPacket(std::client client, std::base_socket::byte_t *Buffer)
//{
//	printf("\npacket %d Buffer: %s", client.port, Buffer);
//}
//
//void ReadPacket(std::server server, std::base_socket::byte_t *Buffer)
//{
//	printf("\npacket %d Buffer: %s", server.port, Buffer);
//}
//
//void ReadPacket(std::server server, std::client client, std::base_socket::byte_t *Buffer)
//{
//	printf("\npacket %d Buffer: %s", server.port, Buffer);
//}
//
//void ReadPacket(std::socket_t Socket, std::base_socket::byte_t *Buffer, uint32_t SizeBuffer)
//{
//	printf("\npacket %d Buffer: %s", Socket, Buffer);
//}
//
//void ReadPacket(std::client client, std::base_socket::byte_t *Buffer, uint32_t SizeBuffer)
//{
//	printf("\npacket %d Buffer: %s", client.port, Buffer);
//}
//
//void ReadPacket(std::server server, std::base_socket::byte_t *Buffer, uint32_t SizeBuffer)
//{
//	printf("\npacket %d Buffer: %s", server.port, Buffer);
//}
//
//void ReadPacket(std::server server, std::client client, std::base_socket::byte_t *Buffer, uint32_t SizeBuffer)
//{
//	printf("\npacket %d Buffer: %s", server.port, Buffer);
//}


int main()
{
	std::base_socket::init_socket();

	std::server server;
	std::client client;

	server["read"] = ReadPacket;
	client["read"] = ReadPacket;

	server.Create("127.0.0.1", 500, std::arch_server_t::tcp_thread);
	server.Run(std::type_blocked_t::non_block);

	client.Connect("127.0.0.1", 500, std::arch_server_t::tcp_thread);
	client.Run(std::type_blocked_t::non_block);

	for (;;) {
		start = std::chrono::high_resolution_clock::now();
		client.Send("Hello, World!\0", 15);

		std::this_thread::sleep_for(1ms);
	}

  

	for (;;)
	{
		Sleep(500);
	}

	return 0;
}

