# SNL - (socket nerwork library). 

std14.

SNL its special library for make just server/client application. SNL support TCP and UDP protocol and used model 1 client 1 thread. For send packet in server or client need binding function in your object by server or client. SNL have 3 base binding function it read, new,  end.

```c++
server["read"] = ReadPacket;
server["new"]  = ConnectClient;
server["end"]  = DisconnectClient;
```

Example hello world code:
```c++
#pragma once

#include <socket.hpp>

using namespace std::chrono_literals;
std::chrono::time_point<std::chrono::steady_clock> start;
std::chrono::time_point<std::chrono::steady_clock> end;

void ReadPacket(std::socket_t Socket, std::base_socket::byte_t *Buffer)
{
	end = std::chrono::high_resolution_clock::now();
	std::chrono::duration<double, std::milli> elapsed = end - start;
	printf("Packet[%d - %lf ms]: %s\n", Socket, elapsed.count(), Buffer);	
}

int main()
{
	std::base_socket::init_socket();

	std::server server;
	std::client client;

	server["read"] = ReadPacket;
	client["read"] = ReadPacket;

	server.create("127.0.0.1", 500, std::arch_server_t::tcp_thread);
	server.run(std::type_blocked_t::non_block);

	client.connect("127.0.0.1", 500, std::arch_server_t::tcp_thread);
	client.run(std::type_blocked_t::non_block);

	for (;;) {
		start = std::chrono::high_resolution_clock::now();
		client.send("Hello, World!\0", 15);

		std::this_thread::sleep_for(1ms);
	}

	return 0;
}
```
SNL support json (by modern json).

```c++
#include <socket.hpp>

using namespace snl;

void json_msg(client c, json &j) {
	for (auto i : json::iterator_wrapper(j))	
		std::cout << i.key() << " " << i.value() << '\n';;
}

int main()
{
	std::system("title Json test");
	std::system("color 06");
	
	server s;
	client c;

	s["json_msg"] = json_msg;

	s.create("127.0.0.1", 12468);
	s.run(type_blocked_t::non_block);

	c.connect("127.0.0.1", 12468);
	c.run(type_blocked_t::non_block);

	std::vector<int> c_vector{ 1, 2, 3, 4 };
	json j_vec(c_vector);

	c.send("json_msg", j_vec);

	json o;
	o["foo"] = 23;
	o["bar"] = false;
	o["baz"] = 3.141;

	c.send("json_msg", o);
	
	for (;;)
	{
		Sleep(1000);
	}

    return 0;
}
```

In test variant i make model multiargument send function, with string/binding function it suppot all types (up to 10 arg). But i have with it problem, because i have undefined behaviour, C++ bad in metaprogramming.

I also plan to expand the number of model server/client types in the future.

