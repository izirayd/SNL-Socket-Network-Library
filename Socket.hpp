/*
  Socket std 2017
*/

#pragma once

#include <functional>
#include <memory>
#include <chrono>
#include <system_error>
#include <stdio.h>
#include <string.h>

#pragma warning(disable : 4996)

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#else
#define PLATFORM_LINUX
#endif

#if defined (PLATFORM_WINDOWS) 
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#if defined (PLATFORM_LINUX)
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#define USE_STD_THREAD

#if defined (PLATFORM_WINDOWS) 
#define wsainit_win                                  \
  int iResult;                                       \
  WSADATA wsaData;                                   \
  iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);    \
  if (iResult != 0) return false;
#endif

#if defined (PLATFORM_LINUX)
#define wsainit_win
#endif


namespace std {

	typedef int32_t socket_t;
	typedef int32_t socket_error;

	class ipv4
	{

#define SYMBOL_END_IP_ZONE '.'

	public:
		ipv4() { Clear(); }
		ipv4(char *ip_src) { this->operator=(ip_src); }
		ipv4(std::string  *ip_src) { this->operator=(ip_src); }
		~ipv4() = default;

		ipv4& operator = (std::string ip_src) {
			this->operator=(ip_src.data());
		}

		ipv4& operator = (char *ip_src) {
			Clear();
			strcpy(this->buf_ip, ip_src);
			Compile();
			return *this;
		}

		bool operator == (ipv4 &src) {
			if (strcmp(this->buf_ip, src.buf_ip) == 0)
				return true;
			return false;
		}

		bool operator >  (ipv4 &src) {

			int32_t my = 0;
			int32_t you = 0;

			for (int32_t i = 0; i < 4; i++)
			{
				my = Parcer(this->buf_ip, i);
				you = Parcer(src.buf_ip, i);

				if (my > you)  return true;
				if (my < you)	return false;
			}

			return false;
		}

		bool operator <  (ipv4 &src) {
			if (this->operator == (src))
				return false;
			return !this->operator > (src);
		}

		ipv4& operator = (ipv4 *ip) {
			Clear();
			strcpy(this->buf_ip, ip->buf_ip);
			return *this;
		}

		bool HostName(char *DomainName) {
			{
				struct hostent *remoteHost;
				struct in_addr addr;

				wsainit_win;

				remoteHost = gethostbyname(DomainName);

				if (remoteHost == NULL)
					return false; else

					addr.s_addr = *(u_long *)remoteHost->h_addr_list[0];

				Clear();

				strcpy(this->buf_ip, inet_ntoa(addr));
				return true;
			}
		}

		void Clear() {
			this->buf_ip[0] = 0x00;
		}

		operator const char*() const {
			return this->buf_ip;
		}

		operator const std::string() const {
			return this->buf_ip;
		}

		int A, B, C, D;
		void Compile() {

			this->A = Parcer(this->buf_ip, 0);
			this->B = Parcer(this->buf_ip, 1);
			this->C = Parcer(this->buf_ip, 2);
			this->D = Parcer(this->buf_ip, 3);

		}

		bool operator >> (ipv4 &src) {


			if (this->A > src.A)
				return true; else if (this->A != src.A) return false;

			if (this->B > src.B)
				return true; else if (this->B != src.B) return false;

			if (this->C > src.C)
				return true; else if (this->C != src.C) return false;

			if (this->D > src.D)
				return true; else if (this->D != src.D)  return false;

			return false;

		}
		bool operator <<  (ipv4 &src) {
			if ((this->A == src.A) && (this->B == src.B) && (this->C == src.C) && (this->D == src.D)) return false;
			return !this->operator >> (src);
		}

		char buf_ip[16];
	private:
		int Parcer(char *src, int type) {
			std::size_t  lenIP = strlen(src);
			int  count_zone = 0;
			char parcerBuf[4];

			for (std::size_t i = 0; i < lenIP; i++) {
				if (count_zone == type) {
					int w = 0;
					while (src[i + w] != SYMBOL_END_IP_ZONE) {
						parcerBuf[w] = src[i + w];
						w++;

						if ((w == 3) || (src[i + w] == 0x00)) {
							parcerBuf[w] = 0x00;
							return atoi(parcerBuf);
						}
					}

					parcerBuf[w] = 0x00;
					return atoi(parcerBuf);
				}

				if (src[i] == SYMBOL_END_IP_ZONE)
					count_zone++;
			}

			return 0;
		}
	};

	namespace base_socket
	{
#if defined(PLATFORM_WINDOWS)
		typedef short       address_family;
		typedef sockaddr_in socket_address_in;
		typedef sockaddr    socket_address;
		typedef int32_t     socket_address_len;
		typedef char        byte_t;
		typedef int32_t     socket_len_t;
#endif

#if defined(PLATFORM_LINUX)
		typedef short       address_family;
		typedef sockaddr_in socket_address_in;
		typedef sockaddr    socket_address;
		typedef uint32_t    socket_address_len;
		typedef uint32_t    socket_len_t;
		typedef char        byte_t;
#endif

#if defined(PLATFORM_WINDOWS)
#define socket_error SOCKET_ERROR
#endif

#if defined(PLATFORM_LINUX)
#define socket_error SO_ERROR
#endif

		enum  class shutdown_t
		{
#if defined(PLATFORM_WINDOWS)
			rd = SD_RECEIVE,
			wr = SD_SEND,
			rdwr = SD_BOTH
#endif
#if defined(PLATFORM_LINUX)
			rd = SHUT_RD,
			wr = SHUT_WR,
			rdwr = SHUT_RDWR
#endif
		};

		int32_t shutdown(std::socket_t socket, shutdown_t sd_option = shutdown_t::rdwr)
		{
			return ::shutdown(socket, static_cast<int32_t>(sd_option));
		}

		int32_t close(std::socket_t socket) {

			base_socket::shutdown(socket);

#if defined(PLATFORM_WINDOWS)
			return ::closesocket(socket);
#endif
#if defined(PLATFORM_LINUX)
			return ::close(socket);
#endif
		}

		bool init_socket() {
			wsainit_win;
			return true;
		}

		enum class type_protocol {
			stream = SOCK_STREAM,
			dgram = SOCK_DGRAM,
			raw = SOCK_RAW,
			rdm = SOCK_RDM,
			seqpacket = SOCK_SEQPACKET
		};

		enum class ipproto {
			unknow = 0,
			ip = IPPROTO_IP,
			icmp = IPPROTO_ICMP,
			igmp = IPPROTO_IGMP,
#if defined(PLATFORM_WINDOWS)
			ggp = IPPROTO_GGP,
			nd = IPPROTO_ND,
#endif
#if defined(PLATFORM_LINUX)
			ggp = IPPROTO_EGP,
			sctp = IPPROTO_SCTP,
#if !defined(__ICC) || !defined(__INTEL_COMPILER)
			//nd = IPPROTO_MH,
#endif
#endif
			tcp = IPPROTO_TCP,
			pup = IPPROTO_PUP,
			udp = IPPROTO_UDP,
			idp = IPPROTO_IDP,
			raw = IPPROTO_RAW,
			max = IPPROTO_MAX,
			echo = IPPORT_ECHO,
			discard = IPPORT_DISCARD,
			systat = IPPORT_SYSTAT,
			daytime = IPPORT_DAYTIME,
			netstat = IPPORT_NETSTAT,
			ftp = IPPORT_FTP,
			telnet = IPPORT_TELNET,
			smtp = IPPORT_SMTP,
			timeserver = IPPORT_TIMESERVER,
			nameserver = IPPORT_NAMESERVER,
			whois = IPPORT_WHOIS,
			mtp = IPPORT_MTP,
			tftp = IPPORT_TFTP,
			rje = IPPORT_RJE,
			finger = IPPORT_FINGER,
			ttylink = IPPORT_TTYLINK,
			supdup = IPPORT_SUPDUP,
			execserver = IPPORT_EXECSERVER,
			loginserver = IPPORT_LOGINSERVER,
			cmdserver = IPPORT_CMDSERVER,
			efsserver = IPPORT_EFSSERVER,
			biffudp = IPPORT_BIFFUDP,
			whoserver = IPPORT_WHOSERVER,
			routerserver = IPPORT_ROUTESERVER,
			reserved = IPPORT_RESERVED
		};

		enum class address_families {
			unspec = 0,
			unix = 1,
			inet = 2,
			implink = 3,
			pup = 4,
			chaos = 5,
			ipx = 6,
			ns = 6,
			iso = 7,
			osi = 7,
			ecma = 8,
			datakit = 9,
			ccitt = 10,
			sna = 11,
			decnet = 12,
			dli = 13,
			lat = 14,
			hylink = 15,
			appletalk = 16,
			netbios = 17,
			voiceview = 18,
			firefox = 19,
			unknown1 = 20,
			ban = 21,
			max = 22
		};

		void CreateAddress(std::base_socket::socket_address_in &address, std::base_socket::address_families family, std::ipv4 ip, int32_t port) { address.sin_family = static_cast<decltype(address.sin_family)>(family); address.sin_addr.s_addr = inet_addr(ip); address.sin_port = htons(port); }

		int32_t bind(std::socket_t socket, std::base_socket::socket_address_in address) {
			return ::bind(socket, (sockaddr*)&address, sizeof(address));
		}

		std::socket_t socket(std::base_socket::address_families address, std::base_socket::type_protocol type_protocol, std::base_socket::ipproto ipproto) {
			return ::socket(static_cast<int>(address), static_cast<int>(type_protocol), static_cast<int>(ipproto));
		}

		int32_t listen(std::socket_t socket, int32_t count) {
			return ::listen(static_cast<int>(socket), count);
		}

		std::socket_t accept(std::socket_t socket, std::base_socket::socket_address_in &address, std::base_socket::socket_address_len &len) {
			return ::accept(static_cast<int>(socket), (struct sockaddr*)&address, &len);
		}

		int32_t recv(std::socket_t socket, std::base_socket::byte_t *Data, std::size_t SizePacket, int32_t Flags) {
			return ::recv(static_cast<int>(socket), static_cast<char *>(Data), static_cast<int>(SizePacket), Flags);
		}

		int32_t send(std::socket_t socket, std::base_socket::byte_t *Data, std::size_t SizePacket, int32_t Flags) {
			return ::send(static_cast<int>(socket), static_cast<char *>(Data), static_cast<int>(SizePacket), Flags);
		}

		int32_t connect(std::socket_t socket, std::base_socket::socket_address_in &address, std::base_socket::socket_address_len &len) {
			return ::connect(static_cast<int>(socket), (struct sockaddr*)&address, len);
		}

		int32_t connect(std::socket_t socket, std::ipv4 ip, int32_t port) {
			std::base_socket::socket_address_in  address;
			std::base_socket::socket_address_len lenAddress = sizeof(address);
			std::base_socket::CreateAddress(address, std::base_socket::address_families::inet, ip, port);
			return std::base_socket::connect(socket, address, lenAddress);
		}

		int32_t connect(std::socket_t socket, std::ipv4 ip, int32_t port, std::base_socket::address_families family) {
			std::base_socket::socket_address_in  address;
			std::base_socket::socket_address_len lenAddress = sizeof(address);
			std::base_socket::CreateAddress(address, family, ip, port);
			return std::base_socket::connect(socket, address, lenAddress);
		}

		int32_t getsockopt(std::socket_t socket, int32_t Level, int32_t Option, std::base_socket::byte_t *Value, socket_len_t &lenValue) {
			return ::getsockopt(static_cast<int>(socket), static_cast<int>(Level), static_cast<int>(Option), Value, &lenValue);
		}

		int32_t setsockopt(std::socket_t socket, int32_t Level, int32_t Option, std::base_socket::byte_t *Value, int32_t lenValue) {
			return ::setsockopt(static_cast<int>(socket), static_cast<int>(Level), static_cast<int>(Option), Value, static_cast<int>(lenValue));
		}

		int32_t reuseaddr(std::socket_t socket) {
			int32_t optval = -1;
			return base_socket::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (std::base_socket::byte_t*) &optval, sizeof(optval));
		}

		int32_t sendto(std::socket_t socket, std::base_socket::byte_t *Data, std::size_t SizePacket, int32_t Flags, std::base_socket::socket_address_in &address, std::base_socket::socket_address_len &len)
		{
			return ::sendto(socket, Data, SizePacket, Flags, (struct sockaddr*)&address, len);
		}

		int32_t sendto(std::socket_t socket, std::base_socket::byte_t *Data, std::size_t SizePacket, int32_t Flags, std::ipv4 ip, int32_t port, std::base_socket::address_families family)
		{
			std::base_socket::socket_address_in  address;
			std::base_socket::socket_address_len len = sizeof(address);
			std::base_socket::CreateAddress(address, family, ip, port);
			return ::sendto(socket, Data, SizePacket, Flags, (struct sockaddr*)&address, len);
		}

		int32_t send(std::socket_t socket, std::base_socket::byte_t *Data, std::size_t SizePacket, int32_t Flags, std::base_socket::socket_address_in &address, std::base_socket::socket_address_len &len)
		{
			return sendto(socket, Data, SizePacket, Flags, address, len);
		}

		int32_t recvfrom(std::socket_t socket, std::base_socket::byte_t *Data, std::size_t SizePacket, int32_t Flags, std::base_socket::socket_address_in &address, std::base_socket::socket_address_len &len)
		{
			return ::recvfrom(socket, Data, SizePacket, Flags, (struct sockaddr*)&address, &len);
		}
	}
}




#if defined(USE_STD_THREAD)
#include <thread>
#else

#define _STD_THREAD_INVALID_HANDLE 0
namespace std
{
	class thread
	{
	public:
		class id
		{
			DWORD mId;
			void clear() { mId = 0; }
			friend class thread;
		public:
			id(DWORD aId = 0) :mId(aId) {}
			bool operator==(const id& other) const { return mId == other.mId; }
		};
	protected:
		HANDLE mHandle;
		id mThreadId;
	public:
		typedef HANDLE native_handle_type;
		id get_id() const noexcept { return mThreadId; }
		native_handle_type native_handle() const { return mHandle; }
		thread() : mHandle(_STD_THREAD_INVALID_HANDLE) {}

		thread(thread&& other)
			:mHandle(other.mHandle), mThreadId(other.mThreadId)
		{
			other.mHandle = _STD_THREAD_INVALID_HANDLE;
			other.mThreadId.clear();
		}

		thread(const thread &other) = delete;

		template<class Function, class... Args>
		explicit thread(Function&& f, Args&&... args)
		{
			typedef decltype(std::bind(f, args...)) Call;
			Call* call = new Call(std::bind(f, args...));
			mHandle = (HANDLE)_beginthreadex(NULL, 0, threadfunc<Call>,
				(LPVOID)call, 0, (unsigned*)&(mThreadId.mId));
		}
		template <class Call>
		static unsigned int __stdcall threadfunc(void* arg)
		{
			std::unique_ptr<Call> upCall(static_cast<Call*>(arg));
			(*upCall)();
			return (unsigned long)0;
		}
		bool joinable() const { return mHandle != _STD_THREAD_INVALID_HANDLE; }
		void join()
		{
			if (get_id() == GetCurrentThreadId())
				throw system_error(EDEADLK, generic_category());
			if (mHandle == _STD_THREAD_INVALID_HANDLE)
				throw system_error(ESRCH, generic_category());
			if (!joinable())
				throw system_error(EINVAL, generic_category());
			WaitForSingleObject(mHandle, INFINITE);
			CloseHandle(mHandle);
			mHandle = _STD_THREAD_INVALID_HANDLE;
			mThreadId.clear();
		}

		~thread()
		{
			if (joinable())
				std::terminate();
		}
		thread& operator=(const thread&) = delete;
		thread& operator=(thread&& other) noexcept
		{
			if (joinable())
				std::terminate();
			swap(std::forward<thread>(other));
			return *this;
		}
		void swap(thread&& other) noexcept
		{
			std::swap(mHandle, other.mHandle);
			std::swap(mThreadId.mId, other.mThreadId.mId);
		}
		static unsigned int hardware_concurrency() noexcept
		{
			static int ncpus = -1;
			if (ncpus == -1)
			{
				SYSTEM_INFO sysinfo;
				GetSystemInfo(&sysinfo);
				ncpus = sysinfo.dwNumberOfProcessors;
			}
			return ncpus;
		}
		void detach()
		{
			if (!joinable())
				throw "";
			if (mHandle != _STD_THREAD_INVALID_HANDLE)
			{
				CloseHandle(mHandle);
				mHandle = _STD_THREAD_INVALID_HANDLE;
			}
			mThreadId.clear();
		}
	};
}
#endif



namespace std {

	class client;
	class server;

	typedef void(*function_socket_byte_t)(std::socket_t, std::base_socket::byte_t *);
	typedef void(*function_client_byte_t)(std::client, std::base_socket::byte_t   *);
	typedef void(*function_server_byte_t)(std::server, std::base_socket::byte_t   *);
	typedef void(*function_server_client_byte_t)(std::server, std::client, std::base_socket::byte_t *);

	typedef void(*function_socket_byte_uint32_t)(std::socket_t, std::base_socket::byte_t *, uint32_t);
	typedef void(*function_client_byte_uint32_t)(std::client, std::base_socket::byte_t   *, uint32_t);
	typedef void(*function_server_byte_uint32_t)(std::server, std::base_socket::byte_t   *, uint32_t);
	typedef void(*function_server_client_byte_uint32_t)(std::server, std::client, std::base_socket::byte_t *, uint32_t);


	template <typename T>
	struct table_function_t
	{
		std::string Name = "";
		T Function = nullptr;
	};

	template <typename T>
	struct list_table_function_t
	{
		list_table_function_t(uint32_t _count)
		{
			if (_count == 0)
				return;

			max = _count;

			function_obj = new table_function_t<T>[max];
		}

		~list_table_function_t()
		{

			if (function_obj != nullptr)
				delete[] function_obj;
		}

		void recreate(uint32_t _count)
		{
			if (_count == 0)
				return;

			max = _count;

			if (function_obj != nullptr)
				delete[] function_obj;


			function_obj = new table_function_t<T>[max];
		}

		uint32_t GetCount() { return count; }
		uint32_t GetMax() { return max; }

		uint32_t count = 0;
		uint32_t max = 0;

		void AddFunction(std::string Name, T Func) {

			if (GetCount() + 1 > GetMax()) return;

			function_obj[count].Name = Name;
			function_obj[count].Function = Func;

			count++;
		}

		T *GetFunction(uint32_t index) {
			if (index > max) return nullptr; function_obj[index];
		}

		T *GetFunction(std::string name) {
			for (uint32_t i = 0; i < count; i++)
				if (function_obj[i].Name == name)
					return &function_obj[i].Function;

			return nullptr;
		}

		table_function_t<T> *function_obj = nullptr;
	};

	struct table_function_socket_byte_t
	{
		char Name[32];
		function_socket_byte_t
			Function;
	};


	class TableFunction
	{
	public:
		TableFunction() {
			this->operator=(32);
		}

		TableFunction(uint32_t CountFunction) {
			this->operator=(CountFunction);
		}

		~TableFunction() = default;

		TableFunction& operator = (const uint32_t ReCreateCount) {

			function_socket_byte.recreate(ReCreateCount);
			function_client_byte.recreate(ReCreateCount);
			function_server_byte.recreate(ReCreateCount);
			function_server_client_byte.recreate(ReCreateCount);
			function_socket_byte_uint32.recreate(ReCreateCount);
			function_client_byte_uint32.recreate(ReCreateCount);
			function_server_byte_uint32.recreate(ReCreateCount);
			function_server_client_byte_uint32.recreate(ReCreateCount);

			return *this;
		}

		void RunForBasePacket(std::string Name, std::socket_t SocketFrom, std::server server, std::client client, std::base_socket::byte_t *Data, uint32_t SizeBuffer);


		void Add(std::string Name, function_socket_byte_t func) { function_socket_byte.AddFunction(Name, func); }
		void Add(std::string Name, function_client_byte_t func) { function_client_byte.AddFunction(Name, func); }
		void Add(std::string Name, function_server_byte_t func) { function_server_byte.AddFunction(Name, func); }
		void Add(std::string Name, function_server_client_byte_t func) { function_server_client_byte.AddFunction(Name, func); }
		void Add(std::string Name, function_socket_byte_uint32_t func) { function_socket_byte_uint32.AddFunction(Name, func); }
		void Add(std::string Name, function_client_byte_uint32_t func) { function_client_byte_uint32.AddFunction(Name, func); }
		void Add(std::string Name, function_server_byte_uint32_t func) { function_server_byte_uint32.AddFunction(Name, func); }
		void Add(std::string Name, function_server_client_byte_uint32_t func) { function_server_client_byte_uint32.AddFunction(Name, func); }

		bool Run(std::string Name, std::socket_t socket, std::base_socket::byte_t *Buffer);
		bool Run(std::string Name, std::client   client, std::base_socket::byte_t *Buffer);
		bool Run(std::string Name, std::server   server, std::base_socket::byte_t *Buffer);
		bool Run(std::string Name, std::server   server, std::client client, std::base_socket::byte_t *Buffer);

		bool Run(std::string Name, std::socket_t socket, std::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(std::string Name, std::client   client, std::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(std::string Name, std::server   server, std::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(std::string Name, std::server   server, std::client client, std::base_socket::byte_t *Buffer, uint32_t SizePacket);

		list_table_function_t<function_socket_byte_t>        function_socket_byte = 0;
		list_table_function_t<function_client_byte_t>        function_client_byte = 0;
		list_table_function_t<function_server_byte_t>        function_server_byte = 0;
		list_table_function_t<function_server_client_byte_t> function_server_client_byte = 0;

		list_table_function_t<function_socket_byte_uint32_t> function_socket_byte_uint32 = 0;
		list_table_function_t<function_client_byte_uint32_t> function_client_byte_uint32 = 0;
		list_table_function_t<function_server_byte_uint32_t> function_server_byte_uint32 = 0;
		list_table_function_t<function_server_client_byte_uint32_t> function_server_client_byte_uint32 = 0;

	};

	enum class status_t {
		success,
		fatal_error,
		error,
		error_create_socket,
		error_bind_socket,
		error_listen_socket,
		error_no_create_socket,
		error_no_select_arch,
		error_connect,
		error_no_send
	};

	enum class arch_server_t
	{
		unknow = 0,
		tcp_thread,               // Каждый клиент имеет свой собственный поток
		tcp_run_thread,           // Каждый пакет будет обработан в созданом для него потоке
		tcp_run_static_thread,    // Каждый пакет будет ждать очереди группы потоков, которая его выполнит
		tcp_run_queue,            // Очередь пакетов будет выполняться в случае если было переданно управление на выполнение этих пакетов каким либо потоком, включая основной(например для обработки сети для через таймер)
		udp_thread,               // Будет исполненно только одним потоком
		udp_run_thread,           // Пакет получит свой поток, в котором он будет выполнен
		udp_run_static_thread,    // Пакет будет отправлен в очередь, пока его не исполнит группа потокок
	    udp_run_queue             // Очередь пакетов будет выполняться в случае если было переданно управление на выполнение этих пакетов каким либо потоком, включая основной(например для обработки сети для через таймер)

	};

	enum class type_blocked_t	{
		non_block,
		block
	};

	class SocketBase
	{
	public:
		SocketBase() {
			TableRunFunction = 32;
		}

		SocketBase(uint32_t CountFunction)
		{
			TableRunFunction = CountFunction;
		}

		~SocketBase() = default;

		void ReadPacketThreadStream(std::socket_t SocketFrom, SocketBase *socket_base_client, SocketBase *socket_base_server);
		void ReadPacketUdp(std::socket_t SocketFrom, SocketBase *socket_base_server);
		void RunPacket(std::string Name, std::socket_t SocketFrom, std::server server, std::client client, std::base_socket::byte_t *Data, uint32_t SizeBuffer);

		void Close() { isRun = false;  std::base_socket::close(socket); }
		void InitAddr() { ip = inet_ntoa(address.sin_addr); port = (int)ntohs(address.sin_port); lenAddress = sizeof(address); }

		SocketBase& operator = (const std::base_socket::socket_address_in _address) { address = _address; InitAddr();  return *this; }
		SocketBase& operator = (const ipv4 _ip) { ip = _ip; return *this; }
		SocketBase& operator = (const std::socket_t _socket) { socket = _socket; return *this; }

		uint32_t SizeRead = 2048;

		bool isRun = true;
		std::socket_t socket = -1;
		std::ipv4     ip = "0.0.0.0";
		int32_t       port   = -1;
		std::base_socket::socket_address_in  address;
		std::base_socket::socket_address_len lenAddress = 0;
		arch_server_t arch = arch_server_t::unknow;
		TableFunction TableRunFunction = 0;
		char SelectFunction[32] = { 0 };
	};

	class server : public SocketBase
	{
	public:

		server() : SizePacketBuffer(1440), SocketBase(32) {
	
		}

		server(std::size_t _SizePacketBuffer, uint32_t CountFunction = 32) : SizePacketBuffer(_SizePacketBuffer), SocketBase(CountFunction) {
		
		}

		std::size_t SizePacketBuffer = 0;

		server &operator[]  (const char *NameFunction)
		{
			strcpy(SelectFunction, NameFunction);
			return *this;
		}

		server& operator = (const function_socket_byte_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_client_byte_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_byte_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_client_byte_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_socket_byte_uint32_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_client_byte_uint32_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_byte_uint32_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_client_byte_uint32_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }

		server& operator = (const SocketBase SB) {

			this->address = SB.address;
			this->arch = SB.arch;
			this->socket = SB.socket;
			this->ip = SB.ip;
			this->socket = SB.socket;
			this->isRun = SB.isRun;
			this->lenAddress = SB.lenAddress;
			this->port = SB.port;
			this->TableRunFunction = SB.TableRunFunction;

			return *this;
		}

		server& operator = (const SocketBase *SB) {

			this->address = SB->address;
			this->arch = SB->arch;
			this->socket = SB->socket;
			this->ip = SB->ip;
			this->socket = SB->socket;
			this->isRun = SB->isRun;
			this->lenAddress = SB->lenAddress;
			this->port = SB->port;
			this->TableRunFunction = SB->TableRunFunction;

			return *this;
		}


		status_t Create(std::ipv4 ipServer, int32_t portServer, std::base_socket::address_families family, std::base_socket::type_protocol type, std::base_socket::ipproto ipproto)
		{
			lenAddress = sizeof(address);
			socket = std::base_socket::socket(family, type, ipproto);

			if (socket == socket_error)
				return status_t::error_create_socket;

			std::base_socket::CreateAddress(address, family, ipServer, portServer);
			int32_t status = std::base_socket::bind(socket, address);

			if (status == socket_error)
				return status_t::error_bind_socket;


			return status_t::success;
		}
		status_t Create(std::ipv4 ipServer, int32_t portServer, arch_server_t archServer)
		{
			if (archServer == arch_server_t::tcp_thread)
			{
				status_t status = Create(ipServer, portServer, std::base_socket::address_families::inet, std::base_socket::type_protocol::stream, std::base_socket::ipproto::tcp);
				if (status != status_t::success)
					return status;

				int32_t statuslisten = std::base_socket::listen(socket, 5);

				if (statuslisten == socket_error)
					return status_t::error_listen_socket;

				arch = archServer;

				return status_t::success;
			}

			if (archServer == arch_server_t::udp_thread)
			{
				status_t status = Create(ipServer, portServer, std::base_socket::address_families::inet, std::base_socket::type_protocol::dgram, std::base_socket::ipproto::udp);

				if (status != status_t::success)
					return status;

				arch = archServer;

				return status_t::success;
			}


			return status_t::error_no_select_arch;
		}
		status_t Run(type_blocked_t type = type_blocked_t::block)
		{
			if (type == type_blocked_t::block)
				return RunServer();

			if (type == type_blocked_t::non_block)
			{
				std::thread threadrun(&server::RunServer, this);
				threadrun.detach();
				return status_t::success;
			}
			return status_t::error_no_select_arch;
		}
		status_t Send(std::socket_t socketClient, std::base_socket::byte_t *Packet, std::size_t SizePacket)
		{
			std::base_socket::send(socketClient, Packet, SizePacket, 0);
		}
	private:
		status_t RunServer()
		{
			if (socket == socket_error)
				return status_t::error_no_create_socket;


			SocketBase *server = new SocketBase(0);
			server->address = this->address;
			server->socket = this->socket;
			server->arch = this->arch;

			if (arch == arch_server_t::tcp_thread)
			{	
				while (true)
				{
					SocketBase *client = new SocketBase(0);
					client->lenAddress = sizeof(client->address);

					client->socket = std::base_socket::accept(socket, client->address, client->lenAddress);
				
					if (client->socket == socket_error)
					{
						delete[] client;
						continue;						
					}

					std::thread   threadclient(&SocketBase::ReadPacketThreadStream, this, client->socket, client, server);
					threadclient.detach();
				}

				delete[] server;
				return status_t::success;
			}

			if (arch == arch_server_t::udp_thread)	
				ReadPacketUdp(socket, server);
			
			delete[] server;
			return status_t::error_no_select_arch;
		}
	};

	class client : public SocketBase
	{
	public:
		client() : SizePacketBuffer(1440), SocketBase(32) {
			CreatePacketBuffer(SizePacketBuffer);
		}
		client(std::size_t _SizePacketBuffer, uint32_t CountFunction = 32) : SizePacketBuffer(_SizePacketBuffer), SocketBase(CountFunction) {		
			CreatePacketBuffer(SizePacketBuffer);
		}

		status_t Connect(std::ipv4 ipServer, int32_t portServer, std::base_socket::address_families family, std::base_socket::type_protocol type, std::base_socket::ipproto ipproto)
		{
			lenAddress = sizeof(address);
			socket = std::base_socket::socket(family, type, ipproto);

			if (socket == socket_error)
				return status_t::error_create_socket;

			std::base_socket::CreateAddress(address, family, ipServer, portServer);
			int32_t
				status = std::base_socket::connect(socket, address, lenAddress);

			if (status == socket_error)
				return status_t::error_connect;

			return status_t::success;
		}
		status_t Connect(std::ipv4 ipServer, int32_t portServer, arch_server_t archServer)
		{
			if (archServer == arch_server_t::tcp_thread)
			{
				status_t status = Connect(ipServer, portServer, std::base_socket::address_families::inet, std::base_socket::type_protocol::stream, std::base_socket::ipproto::tcp);

				if (status != status_t::success)
					return status;

				arch = archServer;

				return status_t::success;
			}

			if (archServer == arch_server_t::udp_thread)
			{
				lenAddress = sizeof(address);
				socket = std::base_socket::socket(std::base_socket::address_families::inet, std::base_socket::type_protocol::dgram, std::base_socket::ipproto::udp);

				if (socket == socket_error)
					return status_t::error_create_socket;

				std::base_socket::CreateAddress(address, std::base_socket::address_families::inet, ipServer, portServer);

				arch = archServer;

				return status_t::success;
			}

			return status_t::error_no_select_arch;
		}
		status_t Run(type_blocked_t type = type_blocked_t::block)
		{
			if (type == type_blocked_t::block)
				return RunClient();

			if (type == type_blocked_t::non_block)
			{
				std::thread threadrun(&client::RunClient, this);
				threadrun.detach();
				return status_t::success;
			}
			return status_t::error_no_select_arch;
		}
		void memcopy(std::base_socket::byte_t* p, std::base_socket::byte_t* b, std::size_t sz) {
			for (size_t i = 0; i < sz; i++)
				p[i] = b[i];
		}

		std::size_t SizePacketBuffer;


		status_t Send(std::base_socket::byte_t *Packet, std::size_t SizePacket)
		{
			if (arch == arch_server_t::tcp_thread)
			{
				memcopy(PacketBuffer, Packet, SizePacket);
				int32_t status = std::base_socket::send(socket, PacketBuffer, SizePacket, 0);
				if (status == socket_error)
					return status_t::error_no_send;
			}

			if (arch == arch_server_t::udp_thread)
			{
				memcopy(PacketBuffer, Packet, SizePacket);
				int32_t status = std::base_socket::send(socket, PacketBuffer, SizePacket, 0, address, lenAddress);

				if (status == socket_error)
					return status_t::error_no_send;
			}

			return status_t::success;
		}



		client &operator[]  (const char *NameFunction)
		{
			strcpy(SelectFunction, NameFunction);
			return *this;
		}

		client& operator = (const function_socket_byte_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		client& operator = (const function_client_byte_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		client& operator = (const function_server_byte_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		client& operator = (const function_server_client_byte_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		client& operator = (const function_socket_byte_uint32_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		client& operator = (const function_client_byte_uint32_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		client& operator = (const function_server_byte_uint32_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }
		client& operator = (const function_server_client_byte_uint32_t Func) { TableRunFunction.Add(SelectFunction, Func);	return *this; }

		client& operator = (const SocketBase SB) {

			this->address = SB.address;
			this->arch = SB.arch;
			this->socket = SB.socket;
			this->ip = SB.ip;
			this->socket = SB.socket;
			this->isRun = SB.isRun;
			this->lenAddress = SB.lenAddress;
			this->port = SB.port;
			this->TableRunFunction = SB.TableRunFunction;

			return *this;
		}

		client& operator = (const SocketBase *SB) {

			this->address = SB->address;
			this->arch = SB->arch;
			this->socket = SB->socket;
			this->ip = SB->ip;
			this->socket = SB->socket;
			this->isRun = SB->isRun;
			this->lenAddress = SB->lenAddress;
			this->port = SB->port;
			this->TableRunFunction = SB->TableRunFunction;

			return *this;
		}

	private:
		status_t RunClient()
		{
			if (socket == socket_error)
				return status_t::error_no_create_socket;

			if (arch == arch_server_t::tcp_thread)
			{
				// Стоит учесть что 0, это количество аллоциеруемых таблиц
				SocketBase *client = new SocketBase(0);
				SocketBase *server = new SocketBase(0);

				client->address = this->address; 
				server->address = this->address;

				client->socket = this->socket;


				std::thread threadclient(&SocketBase::ReadPacketThreadStream, this, socket, client, server);
				threadclient.detach();

				return status_t::success;
			}

			return status_t::error_no_select_arch;
		}
		std::base_socket::byte_t *PacketBuffer = nullptr;
		void CreatePacketBuffer(std::size_t Size) {
			if (PacketBuffer == nullptr && Size > 0)
				PacketBuffer = new std::base_socket::byte_t[Size];
		}
		void DeletePacketBuffer() {
			if (PacketBuffer != nullptr)
			{
				delete[] PacketBuffer;
				PacketBuffer = nullptr;
			}
		}
	};


	bool TableFunction::Run(std::string Name, std::socket_t socket, std::base_socket::byte_t *Buffer)
	{
		function_socket_byte_t *Func = function_socket_byte.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(socket, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(std::string Name, std::client client, std::base_socket::byte_t *Buffer)
	{
		function_client_byte_t *Func = function_client_byte.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(client, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(std::string Name, std::server   server, std::base_socket::byte_t *Buffer)
	{
		function_server_byte_t *Func = function_server_byte.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(server, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(std::string Name, std::server   server, std::client client, std::base_socket::byte_t *Buffer)
	{
		function_server_client_byte_t *Func = function_server_client_byte.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(server, client, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(std::string Name, std::socket_t socket, std::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_socket_byte_uint32_t *Func = function_socket_byte_uint32.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(socket, Buffer, SizePacket));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(std::string Name, std::client   client, std::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_client_byte_uint32_t *Func = function_client_byte_uint32.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(client, Buffer, SizePacket));
			return true;
		}
		return false;
	}

	bool TableFunction::Run(std::string Name, std::server   server, std::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_server_byte_uint32_t *Func = function_server_byte_uint32.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(server, Buffer, SizePacket));

			return true;
		}

		return false;
	}

	bool TableFunction::Run(std::string Name, std::server   server, std::client client, std::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_server_client_byte_uint32_t *Func = function_server_client_byte_uint32.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(server, client, Buffer, SizePacket));
			return true;
		}
		return false;
	}

	void SocketBase::ReadPacketThreadStream(std::socket_t SocketFrom, SocketBase *socket_base, SocketBase *socket_base_server)
	{
		if (socket_base == nullptr || socket_base_server == nullptr || SocketFrom == -1)
			return;

		socket_base->InitAddr();
		socket_base_server->InitAddr();

		std::client client(0, 0);
		std::server server(0, 0);

		client = socket_base;
		server = socket_base_server;

		if (client.socket == socket_error)
			return;

		if (SizeRead == 0)
			return;

		std::base_socket::byte_t *Data = new std::base_socket::byte_t[SizeRead];

		if (Data == nullptr)		{
			printf("\nIn socket library cannot alloc memory for new client.");
			std::base_socket::close(SocketFrom);
			return;
		}

		TableRunFunction.RunForBasePacket("new", SocketFrom, server, client, nullptr, 0);

		while (true) {
			int32_t StatusPacket = std::base_socket::recv(SocketFrom, Data, SizeRead, 0);

			if (StatusPacket > 0)		
				TableRunFunction.RunForBasePacket("read", SocketFrom, server, client, Data, StatusPacket);

			if (!isRun)
			{
				TableRunFunction.RunForBasePacket("closeserver", SocketFrom, server, client, nullptr, 0);
				break;
			}

			if (StatusPacket < 0)
			{
				TableRunFunction.RunForBasePacket("end", SocketFrom, server, client, nullptr, 0);
				break;
			}
		}
		delete socket_base;
		socket_base = nullptr;
		
		delete[] Data;
		Data = nullptr;
	}

	void SocketBase::ReadPacketUdp(std::socket_t SocketFrom, SocketBase *socket_base_server)
	{
		if (socket_base_server == nullptr || SocketFrom == -1)
			return;

		socket_base_server->InitAddr();

		SocketBase socket_base_client(0);
		std::server server(0, 0);
		std::client client(0, 0);


		server = socket_base_server;

		if (SizeRead == 0)
			return;

		std::base_socket::byte_t *Data = new std::base_socket::byte_t[SizeRead];

		if (Data == nullptr) {
			printf("\nIn socket library cannot alloc memory for new client.");
			std::base_socket::close(SocketFrom);
			return;
		}

		while (true) {

			socket_base_client.lenAddress = sizeof(socket_base_client.address);
			int32_t StatusPacket = std::base_socket::recvfrom(SocketFrom, Data, SizeRead, 0, socket_base_client.address, socket_base_client.lenAddress);

			if (StatusPacket < 1)
				continue;

			socket_base_client.InitAddr();

			client = socket_base_client;

			if (StatusPacket > 0)
				TableRunFunction.RunForBasePacket("read", SocketFrom, server, client, Data, StatusPacket);

			if (!isRun)
			{
				TableRunFunction.RunForBasePacket("closeserver", SocketFrom, server, client, nullptr, 0);
				break;
			}

			if (StatusPacket < 0)
			{
				break;
			}
		}

		delete[] Data;
		Data = nullptr;
	}


	void TableFunction::RunForBasePacket(std::string Name, std::socket_t SocketFrom, std::server server, std::client client, std::base_socket::byte_t *Data, uint32_t SizeBuffer)
	{
	
		if (!Run(Name, SocketFrom, Data))
			if (!Run(Name, SocketFrom, Data, SizeBuffer))
				if (!Run(Name, client, Data))
					if (!Run(Name, client, Data, SizeBuffer))
						if (!Run(Name, server, Data))
							if (!Run(Name, server, Data, SizeBuffer))
								if (!Run(Name, server, client, Data))
									  Run(Name, server, client, Data, SizeBuffer);
	}

	void SocketBase::RunPacket(std::string Name, std::socket_t SocketFrom, std::server server, std::client client, std::base_socket::byte_t *Data, uint32_t SizeBuffer)
	{
		if (server.arch == arch_server_t::tcp_thread || server.arch == arch_server_t::udp_thread)
			TableRunFunction.RunForBasePacket(Name, SocketFrom, server, client, Data, SizeBuffer);

	}
}
