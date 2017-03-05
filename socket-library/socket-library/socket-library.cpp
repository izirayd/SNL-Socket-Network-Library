#pragma once

#include "../../Socket.hpp"

#include <iostream>

void ReadPacket(std::socket_t Socket, std::base_socket::byte_t *Buffer)
{
	printf("\npacket %d Buffer: %s", Socket, Buffer);
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

	Sleep(50);
	client.Send("test\0", 5);


	for (;;)
	{
		Sleep(500);
	}

	return 0;
}

