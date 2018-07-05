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

int main()
{
	snl::server server = { "new", newclient };
	snl::client client = { "new", newclient };

	if (server.create("127.0.0.1",  500, snl::arch_server_t::tcp_thread) == snl::status_t::success && server.run(snl::type_blocked_t::non_block) == snl::status_t::success)
    if (client.connect("127.0.0.1", 500, snl::arch_server_t::tcp_thread) == snl::status_t::success && client.run(snl::type_blocked_t::non_block) == snl::status_t::success)
	{ 
	
	}


	for (;;)
		Sleep(400);
	

	return 0;
}