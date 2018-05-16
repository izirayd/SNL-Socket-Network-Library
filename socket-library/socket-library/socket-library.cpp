#pragma once

#if defined _WIN32
#include "../../socket.hpp"
#else
#include "socket.hpp"
#endif

void newclient(nl::client client, nl::base_socket::byte_t *buffer)
{
	printf("%s::%d\n", client.ip.c_str(), client.port);
}

void info()
{
	nl::socket_base_t Base;
	printf("Size info list:\n"
		"Size std::server: %d\nSize std::client: %d\nSize std::socket_base_t: %d\nSocketBase 100000 client: %d MByte\nsocket_address_in: %d\nTableFunction: %d\nstd::list_table_function_t<std::function_server_client_byte_uint32_t>: %d\n", 
		sizeof(nl::server),
		sizeof(nl::client),
		sizeof(nl::socket_base_t),
		sizeof(nl::socket_base_t) * 100000 / 1024 / 1024,
		sizeof(nl::base_socket::socket_address_in),
		sizeof(nl::TableFunction),
		sizeof(nl::list_table_function_t<nl::function_server_client_byte_uint32_t>)
	);
}


int main()
{
	nl::server server = { "new", newclient };
	nl::client client = { "new", newclient };

	if (server.create("127.0.0.1",  500, nl::arch_server_t::tcp_thread) == nl::status_t::success && server.run(nl::type_blocked_t::non_block) == nl::status_t::success)
    if (client.connect("127.0.0.1", 500, nl::arch_server_t::tcp_thread) == nl::status_t::success && client.run(nl::type_blocked_t::non_block) == nl::status_t::success)
	{ 
	
	}


	for (;;)
		Sleep(400);
	

	return 0;
}