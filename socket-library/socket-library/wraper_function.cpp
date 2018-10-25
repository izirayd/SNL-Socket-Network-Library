#pragma once

#if defined _WIN32
#include "../../socket.hpp"
#else
#include "socket.hpp"
#endif

#include <iostream>
#include <chrono>
#include <thread>
#include <atomic>

void foo(int a, int b) { 

	printf("a: %d\n", a);
}

int main()
{


	snl::function_invoke fi;

	fi.AddFunction("foo", foo);
	fi.RunFunction("foo", 10, 15);

	for (;;)
	{
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	return 0;
}