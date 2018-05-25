#pragma once

#if defined _WIN32
#include "../../socket.hpp"
#else
#include "socket.hpp"
#endif

void newclient(snl::client client, snl::base_socket::byte_t *buffer)
{
	printf("%s::%d\n", client.ip.c_str(), client.port);
}

void info()
{
	snl::socket_base_t Base;
	printf("Size info list:\n"
		"Size std::server: %d\nSize std::client: %d\nSize std::socket_base_t: %d\nSocketBase 100000 client: %d MByte\nsocket_address_in: %d\nTableFunction: %d\nstd::list_table_function_t<std::function_server_client_byte_uint32_t>: %d\n", 
		sizeof(snl::server),
		sizeof(snl::client),
		sizeof(snl::socket_base_t),
		sizeof(snl::socket_base_t) * 100000 / 1024 / 1024,
		sizeof(snl::base_socket::socket_address_in),
		sizeof(snl::TableFunction),
		sizeof(snl::list_table_function_t<snl::function_server_client_byte_uint32_t>)
	);
}


int main()
{

	snl::client client;
	snl::socket_base_t socket_base;
	snl::socket_base_t *p = new snl::socket_base_t;


	client = socket_base;
	client = p;

	/*
	snl::server server = { "new", newclient };
	snl::client client = { "new", newclient };

	if (server.create("127.0.0.1",  500, snl::arch_server_t::tcp_thread) == snl::status_t::success && server.run(snl::type_blocked_t::non_block) == snl::status_t::success)
    if (client.connect("127.0.0.1", 500, snl::arch_server_t::tcp_thread) == snl::status_t::success && client.run(snl::type_blocked_t::non_block) == snl::status_t::success)
	{ 
	
	}*/


	for (;;)
		Sleep(400);
	

	return 0;
}