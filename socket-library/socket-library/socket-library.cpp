#pragma once

#if defined _WIN64 || defined _WIN32
#include "../../socket.hpp"
#else
#include "socket.hpp"
#endif

void newclient(std::client client, std::base_socket::byte_t *buffer)
{
	printf("%s", (const char *)client.ip);
}

void info()
{
	std::SocketBase Base;
	printf("Size info list:\n"
		"Size std::server: %d\nSize std::client: %d\nSize std::SocketBase: %d\nSocketBase 100000 client: %d MByte\nsocket_address_in: %d\nTableFunction: %d\nstd::list_table_function_t<std::function_server_client_byte_uint32_t>: %d\n", sizeof(std::server) , sizeof(std::client), sizeof(std::SocketBase), sizeof(std::SocketBase) * 100000 / 1024 / 1024, sizeof(std::base_socket::socket_address_in), sizeof(std::TableFunction), sizeof(std::list_table_function_t<std::function_server_client_byte_uint32_t>)
	);
}

int main()
{
	std::base_socket::init_socket();
	info();
	std::server server = { "new", newclient };
	std::client client = { "new", newclient };;

	if (server.Create("127.0.0.1",  500, std::arch_server_t::tcp_thread) == std::status_t::success && server.Run(std::type_blocked_t::non_block) == std::status_t::success)
    if (client.Connect("127.0.0.1", 500, std::arch_server_t::tcp_thread) == std::status_t::success && client.Run(std::type_blocked_t::block)     == std::status_t::success)
	{ }

	return 0;
}