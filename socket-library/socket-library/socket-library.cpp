#pragma once

#if defined _WIN64 || defined _WIN32
#include "../../Socket.hpp"
#else
#include "Socket.hpp"
#endif

std::chrono::high_resolution_clock::time_point start;
std::chrono::high_resolution_clock::time_point end;

void ReadPacket(std::socket_t Socket, std::base_socket::byte_t *Buffer)
{
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	printf("Packet[%d - %lf ms]: %s\n", Socket, elapsed.count(), Buffer);
}

void NewClient(std::client client, std::base_socket::byte_t *Buffer)
{
	printf("New Client: %s\n", (const char *) client.ip);
}

int main()
{
	printf("\nExample socket hpp library\n");

	std::base_socket::init_socket();

	std::server server;
	std::client client;

	server["read"] = ReadPacket;
	server["new"]  = NewClient;

	std::status_t status = std::status_t::success;

	printf("Start create tcp server for 127.0.0.1::500\n");

	status = server.Create("127.0.0.1", 500, std::arch_server_t::tcp_thread);

	if (status == std::status_t::success)
	{
		printf("Start run server!\n");
		status = server.Run(std::type_blocked_t::non_block);
		if (status == std::status_t::success)
			printf("Server create!\n");
		else
			printf("Error(%d) create server\n", status);

	} else printf("Error(%d) create server\n", status);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	status = client.Connect("127.0.0.1", 500, std::arch_server_t::tcp_thread);


	if (status == std::status_t::success)
		printf("Client Connect!\n");
	else
		printf("Error(%d) connect to server\n", status);

	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	server.ShowError();
	client.ShowError();

	for (;;) {
		start = std::chrono::high_resolution_clock::now();
		client.Send("Hello, World!\0", 15);

		printf("Send packet\n");
	
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}

	return 0;
}