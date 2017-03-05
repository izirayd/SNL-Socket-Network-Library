#pragma once

#include "../../Socket.hpp"
#include <iostream>


using namespace std::chrono_literals;
std::chrono::time_point<std::chrono::steady_clock> start;
std::chrono::time_point<std::chrono::steady_clock> end;

//void ReadPacket(std::socket_t Socket, std::base_socket::byte_t *Buffer)
//{
//	end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double, std::milli> elapsed = end - start;
//	printf("\nPacket[%d - %lf ms]: %s", Socket, elapsed.count(), Buffer);	
//}

//
//void ReadPacket(std::client client, std::base_socket::byte_t *Buffer)
//{
//	end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double, std::milli> elapsed = end - start;
//	printf("\nPacket[%d::%s::%d - %lf ms]: %s", client.socket, (const char*) client.ip, client.port, elapsed.count(), Buffer);
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


	for (;;)	{
		Sleep(500);
	}

	return 0;
}

