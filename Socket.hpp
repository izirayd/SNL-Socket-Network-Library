/*
  Socket snl 2018. Version 0.3 
   
  Socket network library - MIT License.

  Copyright (c) 2018 OrionGame

  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
  including without limitation the rights to use, copy, modify, merge, publish, distribute,
  sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#pragma once

#define socket_major 0
#define socket_minor 3

// socket network library
namespace snl {	
	namespace base_socket	{
		inline int get_major() { return socket_major; }
		inline int get_minor() { return socket_minor; }
	}
}

#include <functional>
#include <memory>
#include <chrono>
#include <system_error>
#include <stdio.h>
#include <string.h>
#include <mutex>
#include <iostream>
#include <vector>
#include <thread>
#include <assert.h>

#pragma warning(disable : 4996)
 
#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(__ILP32__) || defined(_M_X64) || defined(__ia64) || defined(_M_IA64) || defined(__aarch64__) || defined(__powerpc64__) || defined(__amd64)
  #define PLATFORM_ARCH_x64
#endif

// _M_I86 16 bits, but _M_IX86 32 bits
#if defined(__i386__) || (defined(_WIN32) && !defined(_WIN64)) || defined (__i386) || defined(_M_IX86) || defined(__X86__)
  #define PLATFORM_ARCH_x32
#endif

#if defined(__arm__)
  #define PLATFORM_ARCH_ARM
#endif

#if !defined(PLATFORM_ARCH_x64) && !defined(PLATFORM_ARCH_x32) && !defined(PLATFORM_ARCH_ARM)
  #define "No support arch"
#endif

#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM_MAC
#elif defined(__ANDROID__)
#define PLATFORM_ANDROID
#elif defined(__gnu_linux__) || defined(__linux__)
#define PLATFORM_LINUX
#elif
#error "No support for this platform"
#endif

#if defined (PLATFORM_WINDOWS)
#include <process.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#endif

#if defined(PLATFORM_LINUX) && defined(PLATFORM_ANDROID)
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

#define socket_base_mss  1444
#define socket_base_tb   32

#ifdef  __BORLANDC__
  // special operator for run class member
  #define CLOSURE_SUPPORT __closure
#else
  #define CLOSURE_SUPPORT
#endif

// socket network library
namespace snl {
	template<typename T, typename B> inline void memory_cast(T& obj_out, const B obj_in) { memcpy(&obj_out, (unsigned char*)obj_in, sizeof(T)); }
	template<typename T, typename B> inline void memory_cast(T& obj_out, const B& obj_in, const std::size_t size) { memcpy((unsigned char*)obj_out, (const unsigned char*)obj_in, size); }

	void memcopy(char* p, const char* b, std::size_t sz, std::size_t start_position = 0, std::size_t start_position_data = 0) {
		for (size_t i = start_position; i < sz; i++)
			p[i] = b[i + start_position_data];
	}
}

#define operator_add_two_arg(operation)                                                                                                            \
template<typename data_t> friend inline auto operator operation (const uvar&   left, const data_t& right) { return (data_t)left operation right; } \
template<typename data_t> friend inline auto operator operation (const data_t& left, const uvar&   right) { return left operation (data_t)right; } \
                          friend inline uvar operator operation (const uvar&   left, const uvar&   right) { return (std::size_t)left operation (std::size_t)right; }

#define operator_add_one_arg(operation, sub_operation)                                                                                                    \
template<typename data_t, typename = none_t<decay<data_t>>> inline uvar& operator operation (data_t &&Data) { return *this = Data sub_operation *this; }  \
template<typename data_t> friend inline auto operator operation (data_t& left, const uvar& right) { left = left sub_operation(data_t)right;  return left; }

int32_t count_alloc = 0;

// socket network library
namespace snl {

	typedef unsigned char byte_t;

	class uvar
	{
	private:
		byte_t   *Buffer = nullptr;
		std::size_t SizeBuffer = 0;

		template<typename T>   using decay = typename std::decay<T>::type;
		template<typename T>   using none_t = typename std::enable_if<!std::is_same<uvar, T>::value>::type;

		inline bool alloc(std::size_t size_alloc) {
			if (size_alloc == 0 || Buffer != nullptr)  return false;
			SizeBuffer = size_alloc;
			Buffer = new byte_t[size_alloc];
			//printf("alloc %d number alloc: %d\n", size_alloc, count_alloc);
			//count_alloc++;
			return true;
		}
		inline void free() {
			if (Buffer != nullptr) {
				delete[] Buffer;
				SizeBuffer = 0;
				Buffer = nullptr;
			}
		}
		void realloc(std::size_t new_size) {
			if (new_size <= SizeBuffer)
				return;
			byte_t *tmpBuffer = new byte_t[SizeBuffer];
			snl::memory_cast(tmpBuffer, Buffer, SizeBuffer);
			free();
			alloc(new_size);
			snl::memory_cast(Buffer, tmpBuffer, SizeBuffer);
			delete[] tmpBuffer;
		}

	public:
		~uvar() { free(); }
	//	uvar() = default;
		uvar(std::nullptr_t null = nullptr) {}
		template<typename data_t, typename = none_t<decay<data_t>>>
		uvar(data_t&& Data) { if (alloc(sizeof(data_t)))      snl::memory_cast(Buffer, &Data, SizeBuffer); }
		uvar(uvar &&VarData) { if (alloc(VarData.SizeBuffer))  snl::memory_cast(this->Buffer, VarData.Buffer, SizeBuffer); }
		uvar(uvar const &VarData) { 
			if (alloc(VarData.SizeBuffer)) 
				snl::memory_cast(this->Buffer, VarData.Buffer, SizeBuffer); 
		}

		uvar &operator=(uvar &&VarData) {
			if (this->SizeBuffer == VarData.SizeBuffer || this->SizeBuffer > VarData.SizeBuffer) {
				snl::memory_cast(this->Buffer, VarData.Buffer, SizeBuffer);
				return *this;
			}
			free();
			if (alloc(VarData.SizeBuffer))
			{
				snl::memory_cast(this->Buffer, VarData.Buffer, SizeBuffer);
				return *this;
			}
			return *this;
		}

		uvar &operator=(uvar const &VarData) {
			if (this->SizeBuffer == VarData.SizeBuffer || this->SizeBuffer > VarData.SizeBuffer) {
				snl::memory_cast(this->Buffer, VarData.Buffer, SizeBuffer);
				return *this;
			}
			free();
			if (alloc(VarData.SizeBuffer)) {
				snl::memory_cast(this->Buffer, VarData.Buffer, SizeBuffer);
				return *this;
			}
			return *this;
		}

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline void memory_get(data_t&& Data, const std::size_t sizeData) {
			if (sizeData == 0)
				return;
			if (sizeData >= SizeBuffer)
				snl::memory_cast(Data, &Buffer, SizeBuffer); // В случае чего & уберается. Так и не пофиксил похоже этот баг
		}

		template<typename B>
		inline void memory_add(const B& Data, const std::size_t sizeData) {
			if (sizeData == 0)
				return;
			if (sizeData == SizeBuffer) {
				snl::memory_cast(this->Buffer, Data, SizeBuffer);
				return;
			}
			free();
			alloc(sizeData);
			snl::memory_cast(this->Buffer, Data, SizeBuffer);
		}

		template<typename data_t, typename = none_t<decay<data_t>>>
		uvar &operator=(data_t &&Data) {

			std::size_t sizeData = sizeof(data_t);
			if (sizeData == 0)
				return *this;

			if (sizeData == SizeBuffer) {
				snl::memory_cast(this->Buffer, &Data, SizeBuffer);
				return *this;
			}

			free();
			alloc(sizeData);
			snl::memory_cast(this->Buffer, &Data, SizeBuffer);
			return *this;
		}

		inline bool operator== (const snl::uvar &Data) {
			if (this->size() == Data.size()) {
				for (size_t i = 0; i < this->size(); i++)
					if (this->Buffer[i] != Data.Buffer[i])
						return false;
				return true;
			}
			return false;
		}

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline bool operator== (data_t &&Data) {
			const data_t &tmp = *this;
			if (tmp == Data)
				return true;
			return false;
		}

		inline operator const char* () const & { return (char*)(Buffer); }
		inline operator char* () const & { return (char*)(Buffer); }

		inline operator const unsigned char* () const & { return (unsigned char*)(Buffer); }
		inline operator unsigned char* () const & { return (unsigned char*)(Buffer); }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline bool operator!= (data_t &&Data) { return !this->operator==(Data); }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline operator data_t const & () const & { return *(data_t*)(Buffer); }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline operator data_t && () && {   return  reinterpret_cast<data_t>(Buffer);	  }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline operator data_t       & ()       & { return  *reinterpret_cast<data_t*>(Buffer); }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline operator data_t       * () { return  reinterpret_cast<data_t*>(Buffer); }

		inline uvar  operator ++ (int) { uvar result(*this);          ++(*this);  return result; }
		inline uvar  operator -- (int) { uvar result(*this);          --(*this);  return result; }
		inline uvar& operator ++ () { int &tmp = *this;            tmp++;      return *this; }
		inline uvar& operator -- () { std::size_t &tmp = *this;    tmp--;      return *this; }

		operator_add_two_arg(+);
		operator_add_two_arg(-);
		operator_add_two_arg(/ );
		operator_add_two_arg(*);
		operator_add_two_arg(%);
		operator_add_two_arg(>);
		operator_add_two_arg(<);
		operator_add_two_arg(>=);
		operator_add_two_arg(<=);
		operator_add_two_arg(==);
		operator_add_two_arg(&&);
		operator_add_two_arg(|| );
		operator_add_two_arg(&);
		operator_add_two_arg(| );
		operator_add_two_arg(^);
		operator_add_one_arg(+=, +);
		operator_add_one_arg(-=, -);
		operator_add_one_arg(*=, *);
		operator_add_one_arg(/=, / );
		operator_add_one_arg(%=, %);
		operator_add_one_arg(&=, &);
		operator_add_one_arg(|=, | );
		operator_add_one_arg(^=, ^);

		inline int8_t   ToInt8() { return (int8_t)*this; }
		inline int16_t  ToInt16() { return (int16_t)*this; }
		inline int32_t  ToInt32() { return (int32_t)*this; }
		inline int64_t  ToInt64() { return (int64_t)*this; }
		inline uint8_t  ToUInt8() { return (uint8_t)*this; }
		inline uint16_t ToUInt16() { return (uint16_t)*this; }
		inline uint32_t ToUInt32() { return (uint32_t)*this; }
		inline uint64_t ToUInt64() { return (uint64_t)*this; }
		inline char*    ToChar() { return (char*)*this; }
		inline byte_t*  ToByte() { return (byte_t*)*this; }
		inline float_t  ToFloat() { return (float_t)*this; }

		inline std::size_t &size()  { return SizeBuffer; }
		inline const std::size_t size() const { return SizeBuffer; }
		inline const std::size_t size(std::size_t set_size) { SizeBuffer = set_size; return SizeBuffer; }
	};

	class stream_buffer;

	template<typename ValueType>
	class stream_buffer_iterator : private std::iterator<std::input_iterator_tag, ValueType>
	{
		friend class stream_buffer;
	private:
		stream_buffer_iterator(ValueType* p) : p(p) {}
	public:
		stream_buffer_iterator(const stream_buffer_iterator &it) : p(it.p) {}
		bool operator!=(stream_buffer_iterator const& other) {
			if (isLast)       isShow = false;
			if (p == other.p) isLast = true;
			return isShow;
		}
		bool operator==(stream_buffer_iterator const& other) const { return p == other.p; }
		typename stream_buffer_iterator::reference operator*() const { return *p; }
		inline stream_buffer_iterator& operator++() { ++p; return *this; }
	private:
		ValueType* p;
		bool isShow = true;
		bool isLast = false;
	};

	class stream_buffer
	{
		template<typename T>
		using decay = typename std::decay<T>::type;
		template<typename T>
		using none_t = typename std::enable_if<!std::is_same<stream_buffer, T>::value>::type;

	public:
		stream_buffer() = default;

		/*  template<typename data_t, typename = none_t<decay<data_t>>>
		stream_buffer(data_t&& Data) { }
		stream_buffer(stream_buffer &&VarData)      { *this = &VarData; } // TODO: Проверить, потому что если VarData удалится, класс повредится и еще чекнуть на деструктор
		stream_buffer(stream_buffer const &VarData) { *this = &VarData; }*/

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline void  push_back(data_t &&Data) { *this << Data; }

		inline void  clear() { blockPos = -1;  BlockData.clear(); }
		inline bool  isEmpty() { return BlockData.empty(); }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline const data_t& back() const { return BlockData[BlockData.size() - 1]; }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline data_t&       back() { return BlockData[BlockData.size() - 1]; }

		inline void  pop_back() { blockPos--; BlockData.pop_back(); }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline data_t&  front() { return BlockData[0]; }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline const data_t&  front() const { return BlockData[0]; }

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline stream_buffer &operator << (data_t &&Data) {
			BlockData.push_back(Data);
			blockPos++;
			return *this;
		}


		/*  template<typename data_t, typename = none_t<decay<data_t>>>
		inline stream_buffer &operator=(data_t &&Data) {
		BlockData.push_back(Data);
		blockPos++;
		return *this;
		}
		*/
		template<typename data_t, typename = none_t<decay<data_t>>>
		inline stream_buffer &operator >> (data_t &&Data) {
			if (blockPos >= 0)
			{
				//BlockData[blockPos--].memory_add(&Data, sizeof(data_t));

				//  BlockData[blockPos--].operator=(Data);
				BlockData[blockPos--].memory_get(&Data, sizeof(data_t));
			}
			return *this;
		}

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline operator data_t       * () {
			if (blockPos >= 0)
				return BlockData[blockPos--];
			return nullptr;
		}

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline operator data_t && () {
			if (blockPos >= 0)
				return BlockData[blockPos--];
			return nullptr;
		}


		/* inline  snl::uvar*   operator[] (std::vector<snl::uvar>::size_type pos) {
		return BlockData[pos];
		}*/


		inline   snl::uvar&  operator[] (std::vector<snl::uvar>::size_type pos) {
			return BlockData[pos];
		}


		template<typename data_t>
		inline  data_t*   at(std::vector<snl::uvar>::size_type pos) {
			return BlockData[pos];
		}

		template<typename data_t>
		inline  data_t&&  at(std::vector<snl::uvar>::size_type pos) {
			return BlockData[pos];
		}

		typedef stream_buffer_iterator<snl::uvar> iterator;
		typedef stream_buffer_iterator<snl::uvar> const_iterator;

		inline iterator       begin() { return &BlockData[0]; }
		inline iterator       end() { return &BlockData[BlockData.size() - 1]; }
		inline const const_iterator cbegin() { return  BlockData[0]; }
		inline const const_iterator cend() { return  BlockData[BlockData.size() - 1]; }

		inline  std::vector<snl::uvar>::size_type size() const { return BlockData.size(); }
		inline  std::vector<snl::uvar>::size_type max_size() const { return BlockData.max_size(); }
		inline  std::vector<snl::uvar>::size_type capacity() const { return BlockData.capacity(); }
		inline void shrink_to_fit() { BlockData.shrink_to_fit(); }

		inline void reserve(std::vector<snl::uvar>::size_type size) { BlockData.reserve(size); }
		inline void resize(std::vector<snl::uvar>::size_type size)  { BlockData.resize(size); }

		template<typename ... Args>
		inline void push_back(Args&& ... args) { arg_context(add_context(args)...); }

		inline snl::uvar *data() {
			return BlockData
				.data();
		}

		inline const snl::uvar *data() const {
			return BlockData
				.data();
		}

		void data_allbuffer(char *Buffer)
		{
			int32_t size_list = this->size();
			snl::memory_cast(Buffer, &size_list, sizeof(int32_t));
			Buffer += sizeof(int32_t);
			for (auto &it : *this) {
				snl::memory_cast(Buffer, &it.size(), sizeof(int32_t));
				Buffer += sizeof(int32_t);
				snl::memory_cast(Buffer, it.ToChar(), it.size());
				Buffer += it.size();
			}
		}

		void data_allbuffer_read(char *Buffer)
		{
			int32_t size = 0;		
			memcpy(&size, Buffer, sizeof(int32_t));

			if (size != 0 && size > 0)
			{
				this->reserve(size);
				this->resize(size);
			}
			else return;

			Buffer += sizeof(int32_t);

			for (auto &it : *this) {
				memcpy(&size, Buffer, sizeof(int32_t));
				Buffer += sizeof(int32_t);
				it.memory_add(Buffer, size);
				Buffer += size;
			}
		}

	private:
		std::vector<snl::uvar> BlockData;
		std::intptr_t blockPos = -1;

		template<typename data_t, typename = none_t<decay<data_t>>>
		inline data_t add_context(data_t&& Data) { this->operator << (Data); return Data; }

		template<typename... Args> inline	void arg_context(Args&& ... args) {}
	};
}

// socket network library
namespace snl
{
	struct function_uvar_t
	{
		virtual ~function_uvar_t() = default;

		virtual void call(
			const uint32_t &count
			, const snl::uvar& a1  = snl::uvar{ nullptr }
			, const snl::uvar& a2  = snl::uvar{ nullptr }
			, const snl::uvar& a3  = snl::uvar{ nullptr }
			, const snl::uvar& a4  = snl::uvar{ nullptr }
			, const snl::uvar& a5  = snl::uvar{ nullptr }
			, const snl::uvar& a6  = snl::uvar{ nullptr }
			, const snl::uvar& a7  = snl::uvar{ nullptr }
			, const snl::uvar& a8  = snl::uvar{ nullptr }
			, const snl::uvar& a9  = snl::uvar{ nullptr }
			, const snl::uvar& a10 = snl::uvar{ nullptr }
		) = 0;
};

	template <typename... Args>
	class base_function_wraper : public function_uvar_t
	{
	public:
		base_function_wraper(std::function<void(Args...)> f) : _f{ f } { count_arg = sizeof...(Args);}

		virtual void call(
			const uint32_t &count
			, const snl::uvar& a1  = snl::uvar{ nullptr }
			, const snl::uvar& a2  = snl::uvar{ nullptr }
			, const snl::uvar& a3  = snl::uvar{ nullptr }
			, const snl::uvar& a4  = snl::uvar{ nullptr }
			, const snl::uvar& a5  = snl::uvar{ nullptr }
			, const snl::uvar& a6  = snl::uvar{ nullptr }
			, const snl::uvar& a7  = snl::uvar{ nullptr }
			, const snl::uvar& a8  = snl::uvar{ nullptr }
			, const snl::uvar& a9  = snl::uvar{ nullptr }
			, const snl::uvar& a10 = snl::uvar{ nullptr }
		) override
		{
			if (count == count_arg)
				call_func(std::make_tuple(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10), std::make_index_sequence<sizeof...(Args)>{});
		}

		uint32_t count_arg = 0;

	private:
		template <typename Tuple, std::size_t... Indices>
		inline void call_func(const Tuple& t, const std::index_sequence<Indices...> &s) { 
			_f(std::get<Indices>(t)...); 
		}
		std::function<void(Args...)> _f;
	};

	template <typename... Args>	inline std::shared_ptr<function_uvar_t> make_point_wp(void(*f)(Args...)) { return std::make_shared<base_function_wraper<Args...>>(f); }

	class function_invoke
	{
	public:
		template<typename _FObj>
		void AddFunction(const std::string& name, const _FObj & fun) {
			FunctionList.push_back(make_point_wp(&fun));
			NameList.push_back(name);
		}

		void RunFunction(const std::string& name) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(0);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1>
		void RunFunction(const std::string& name, const TypeArg1& arg1) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(1, arg1);
					break;
				}
				index++;
		   }
		}

		template<typename TypeArg1, typename TypeArg2>
		void RunFunction(const std::string& name, const TypeArg1& arg1, const TypeArg2& arg2) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(2, arg1, arg2);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1, typename TypeArg2, typename TypeArg3>
		void RunFunction(const std::string& name, const TypeArg1& arg1, const TypeArg2& arg2, const TypeArg3& arg3) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(3, arg1, arg2, arg3);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1, typename TypeArg2, typename TypeArg3, typename TypeArg4>
		void RunFunction(const std::string& name, const TypeArg1& arg1, const TypeArg2& arg2, const TypeArg3& arg3, const TypeArg4& arg4) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(4, arg1, arg2, arg3, arg4);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1, typename TypeArg2, typename TypeArg3, typename TypeArg4, typename TypeArg5>
		void RunFunction(const std::string& name, const TypeArg1& arg1, const TypeArg2& arg2, const TypeArg3& arg3, const TypeArg4& arg4, const TypeArg5& arg5) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(5, arg1, arg2, arg3, arg4, arg5);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1, typename TypeArg2, typename TypeArg3, typename TypeArg4, typename TypeArg5,
			typename TypeArg6>
			void RunFunction(const std::string& name, const TypeArg1& arg1, const TypeArg2& arg2, const TypeArg3& arg3, const TypeArg4& arg4, const TypeArg5& arg5, const TypeArg6& arg6) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(6, arg1, arg2, arg3, arg4, arg5, arg6);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1, typename TypeArg2, typename TypeArg3, typename TypeArg4, typename TypeArg5,
			typename TypeArg6, typename TypeArg7>
			void RunFunction(const std::string& name, const TypeArg1& arg1, const TypeArg2& arg2, const TypeArg3& arg3, const TypeArg4& arg4, const TypeArg5& arg5, const TypeArg6& arg6, const TypeArg7& arg7) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(7, arg1, arg2, arg3, arg4, arg5, arg6, arg7);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1, typename TypeArg2, typename TypeArg3, typename TypeArg4, typename TypeArg5,
			typename TypeArg6, typename TypeArg7, typename TypeArg8>
			void RunFunction(const std::string& name, const TypeArg1& arg1, const TypeArg2& arg2, const TypeArg3& arg3, const TypeArg4& arg4, const TypeArg5& arg5, const TypeArg6& arg6, const TypeArg7& arg7, const TypeArg8& arg8) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(8, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1, typename TypeArg2, typename TypeArg3, typename TypeArg4, typename TypeArg5,
			typename TypeArg6, typename TypeArg7, typename TypeArg8, typename TypeArg9>
			void RunFunction(const std::string& name, const TypeArg1 arg1, const TypeArg2& arg2, const TypeArg3& arg3, const TypeArg4& arg4, const TypeArg5& arg5, const TypeArg6& arg6, const TypeArg7& arg7, const TypeArg8& arg8, const TypeArg9& arg9) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(9, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);
					break;
				}
				index++;
			}
		}

		template<typename TypeArg1, typename TypeArg2, typename TypeArg3, typename TypeArg4, typename TypeArg5,
			typename TypeArg6, typename TypeArg7, typename TypeArg8, typename TypeArg9, typename TypeArg10>
			void RunFunction(const std::string& name, const TypeArg1& arg1, const TypeArg2& arg2, const TypeArg3& arg3, const TypeArg4& arg4, const TypeArg5& arg5, const TypeArg6& arg6, const TypeArg7& arg7, const TypeArg8& arg8, const TypeArg9& arg9, const TypeArg10& arg10) {
			int32_t index = 0;
			for (auto &itName : NameList) {
				if (itName == name) {
					FunctionList[index]->call(10, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
					break;
				}
				index++;
			}
		}

		std::vector<std::shared_ptr<function_uvar_t>> FunctionList;
		std::vector<std::string> NameList;
	};
}

// socket network library
namespace snl {

#if defined(PLATFORM_ARCH_x64) && defined(PLATFORM_WINDOWS)
	typedef uint64_t   socket_t;
#elif defined(PLATFORM_WINDOWS) && defined(PLATFORM_ARCH_x32)
	typedef uint32_t   socket_t;
#elif  defined(PLATFORM_LINUX)
	typedef int32_t    socket_t;
#endif
	
	typedef int32_t  socket_error;
	typedef uint16_t port_t;

	enum class packet_header_t
	{
		wrapertable_packet = 0,
		wrapertable_arg_packet = 1
	};


	class ipv4
	{
	public:
		ipv4() { Clear(); }
		ipv4(const char *ip_src)   { operator=(ip_src); }
		ipv4(std::string  *ip_src) { operator=(ip_src); }
		~ipv4() = default;

		ipv4& operator = (const std::string ip_src) { this->operator=(ip_src.data()); }
		ipv4& operator = (const char *ip_src) {
			Clear();
			strcpy(ip, ip_src);
			Compile();
			return *this;
		}

		inline bool operator == (const ipv4 &src) {
			if (strcmp(ip, src.ip) == 0)
				return true;
			return false;
		}

		inline bool operator >  (ipv4 &src) {

			int32_t my = 0;
			int32_t your = 0;

			for (int32_t i = 0; i < 4; i++)
			{
				my = Parcer(ip, i);
				your = Parcer(src.ip, i);

				if (my > your)  return true;
				if (my < your)	return false;
			}

			return false;
		}

		bool operator <  (ipv4 &src) {
			if (this->operator == (src))
				return false;
			return !this->operator > (src);
		}

		ipv4& operator = (const ipv4 *_ip) {
			Clear();
			strcpy(ip, _ip->ip);
			return *this;
		}

		inline uint32_t inet_addres() { return ::inet_addr(*this); }

		bool HostName(const char *DomainName) {

			if (DomainName == NULL || DomainName == nullptr || DomainName[0] == 0x00)
				return false;

			struct hostent *remoteHost;
			struct in_addr addr;

			remoteHost = gethostbyname(DomainName);

			if (remoteHost == NULL)
				return false; else

				addr.s_addr = *(u_long *)remoteHost->h_addr_list[0];

			Clear();

			strcpy(ip, inet_ntoa(addr));
			return true;
		}

		inline void           Clear() { ip[0] = 0x00; }
		inline operator const char*() const { return ip; }
		inline operator const std::string() const { return ip; }

		uint8_t A, B, C, D;
		void Compile() {
			A = Parcer(ip, 0);
			B = Parcer(ip, 1);
			C = Parcer(ip, 2);
			D = Parcer(ip, 3);
		}

		bool operator >> (const ipv4 src) const {

			if (A > src.A)
				return true; else if (A != src.A) return false;
			if (B > src.B)
				return true; else if (B != src.B) return false;
			if (C > src.C)
				return true; else if (C != src.C) return false;
			if (D > src.D)
				return true; else if (D != src.D)  return false;

			return false;
		}

		inline bool operator << (const ipv4 src) const {	return ((A == src.A) && (B == src.B) && (C == src.C) && (D == src.D)) ? false : !operator >> (src);	}

		inline const char *c_str() const { return ip; }
		inline const char *get() const { return ip; }
		char ip[16];

	private:
		uint8_t Parcer(const char *src, const int32_t type) {

			std::size_t  lenIP = strlen(src);
			int  count_zone = 0;
			char parcerBuf[4];

			for (std::size_t i = 0; i < lenIP; i++) {
				if (count_zone == type) {
					int w = 0;
					while (src[i + w] != '.') {
						parcerBuf[w] = src[i + w];
						w++;

						if ((w == 3) || (src[i + w] == 0x00)) {
							parcerBuf[w] = 0x00;
							return static_cast<uint8_t>(atoi(parcerBuf));
						}
					}

					parcerBuf[w] = 0x00;
					return static_cast<uint8_t>(atoi(parcerBuf));
				}

				if (src[i] == '.')
					count_zone++;
			}

			return 0;
		}
	};

	namespace base_socket
	{
		typedef short       address_family;
		typedef sockaddr_in socket_address_in;
		typedef sockaddr    socket_address;

		typedef char                 byte_t;
		typedef unsigned char        ubyte_t;


#if defined(PLATFORM_WINDOWS)		
		typedef int32_t     socket_address_len;
		typedef int32_t     socket_len_t;
#endif

#if defined(PLATFORM_LINUX)		
		typedef uint32_t    socket_address_len;
		typedef uint32_t    socket_len_t;
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

		inline int32_t shutdown(snl::socket_t socket, shutdown_t sd_option = shutdown_t::rdwr)
		{
			return ::shutdown(socket, static_cast<int32_t>(sd_option));
		}

		inline int32_t close(snl::socket_t socket) {

			base_socket::shutdown(socket);

#if defined(PLATFORM_WINDOWS)
			return ::closesocket(socket);
#endif
#if defined(PLATFORM_LINUX)
			return ::close(socket);
#endif
		}
		
		static bool is_init_socket = false;

		inline bool init_socket() {

#if defined (PLATFORM_WINDOWS)
			if (!is_init_socket) {

				WSADATA wsaData;                                   
				if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)	{
					assert(false && "Cannot init win-socket library!");
					return false;
				}		
				is_init_socket = true;
			}
#endif
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

		enum class address_families : int32_t {
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
			atm = 22,
			inet6 = 23,
			cluster = 24,
			af12844 = 25,
			irda = 26,
			netdes = 28,
			tcnproc = 29,
			tcnmess = 30,
			iclfxbm = 31,
			bth = 32,
			link = 33,
			max
		};

		inline uint32_t    inet_addres(snl::ipv4 ip) { return ::inet_addr(ip); }

		inline uint16_t ntohs(uint16_t us) { return ::ntohs(us); }
		inline uint16_t htons(uint16_t us) { return ::htons(us); }
		inline uint32_t htonl(uint32_t ui) { return ::htonl(ui); }
		inline uint32_t ntohl(uint32_t ui) { return ::ntohl(ui); }

		inline void create_address(snl::base_socket::socket_address_in &address, snl::base_socket::address_families family, snl::ipv4 ip, snl::port_t port) {
			init_socket();
			address.sin_family = static_cast<decltype(address.sin_family)>(family); address.sin_addr.s_addr = snl::base_socket::inet_addres(ip); address.sin_port = snl::base_socket::htons(port);
		}		
			
		inline int32_t bind(snl::socket_t socket, const snl::base_socket::socket_address_in address) {
			init_socket();
			return ::bind(socket, (sockaddr*)&address, sizeof(address));
		}

		inline snl::socket_t socket(snl::base_socket::address_families address, snl::base_socket::type_protocol type_protocol, snl::base_socket::ipproto ipproto) {
			init_socket();
			return ::socket(static_cast<int>(address), static_cast<int>(type_protocol), static_cast<int>(ipproto));
		}

		inline int32_t listen(snl::socket_t socket, int32_t count) {
			init_socket();
			return ::listen(socket, count);
		}

		inline snl::socket_t accept(snl::socket_t socket, snl::base_socket::socket_address_in &address, snl::base_socket::socket_address_len &len) {
			init_socket();
			return ::accept(socket, (struct sockaddr*)&address, &len);
		}

		inline int32_t recv(snl::socket_t socket, snl::base_socket::byte_t *Data, int32_t SizePacket, int32_t Flags) {
			return ::recv(socket, static_cast<char *>(Data), SizePacket, Flags);
		}

		inline int32_t send(snl::socket_t socket, const snl::base_socket::byte_t *Data, int32_t SizePacket, int32_t Flags) {
			return ::send(socket, static_cast<const char *>(Data), SizePacket, Flags);
		}

		inline int32_t connect(snl::socket_t socket, const snl::base_socket::socket_address_in &address, snl::base_socket::socket_address_len &len) {
			init_socket();
			return ::connect(socket, (struct sockaddr*)&address, len);
		}

		inline int32_t connect(snl::socket_t socket, snl::ipv4 ip, snl::port_t port) {
			init_socket();
			snl::base_socket::socket_address_in  address;
			snl::base_socket::socket_address_len lenAddress = sizeof(address);
			snl::base_socket::create_address(address, snl::base_socket::address_families::inet, ip, port);
			return snl::base_socket::connect(socket, address, lenAddress);
		}

		inline int32_t connect(snl::socket_t socket, snl::ipv4 ip, snl::port_t port, snl::base_socket::address_families family) {
			init_socket();
			snl::base_socket::socket_address_in  address;
			snl::base_socket::socket_address_len lenAddress = sizeof(address);
			snl::base_socket::create_address(address, family, ip, port);
			return snl::base_socket::connect(socket, address, lenAddress);
		}

		inline int32_t getsockopt(snl::socket_t socket, int32_t Level, int32_t Option, snl::base_socket::byte_t *Value, socket_len_t &lenValue) {
			init_socket();
			return ::getsockopt(socket, static_cast<int>(Level), static_cast<int>(Option), Value, &lenValue);
		}

		inline int32_t setsockopt(snl::socket_t socket, int32_t Level, int32_t Option, snl::base_socket::byte_t *Value, int32_t lenValue) {
			init_socket();
			return ::setsockopt(socket, static_cast<int>(Level), static_cast<int>(Option), Value, static_cast<int>(lenValue));
		}

		inline int32_t reuseaddr(snl::socket_t socket) {
			init_socket();
			int32_t optval = -1;
			return base_socket::setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (snl::base_socket::byte_t*) &optval, sizeof(optval));
		}

		inline int32_t sendto(snl::socket_t socket, const snl::base_socket::byte_t *Data, int32_t SizePacket, int32_t Flags, snl::base_socket::socket_address_in &address, snl::base_socket::socket_address_len &len) {
			return ::sendto(socket, Data, SizePacket, Flags, (struct sockaddr*)&address, len);
		}

		inline int32_t sendto(snl::socket_t socket, snl::base_socket::byte_t *Data, int32_t SizePacket, int32_t Flags, snl::ipv4 ip, snl::port_t port, snl::base_socket::address_families family) {
			snl::base_socket::socket_address_in  address;
			snl::base_socket::socket_address_len len = sizeof(address);
			snl::base_socket::create_address(address, family, ip, port);
			return ::sendto(socket, Data, SizePacket, Flags, (struct sockaddr*)&address, len);
		}

		inline int32_t send(snl::socket_t socket, const snl::base_socket::byte_t *Data, int32_t SizePacket, int32_t Flags, snl::base_socket::socket_address_in &address, snl::base_socket::socket_address_len &len) {
			return sendto(socket, Data, SizePacket, Flags, address, len);
		}

		inline int32_t recvfrom(snl::socket_t socket, snl::base_socket::byte_t *Data, int32_t SizePacket, int32_t Flags, snl::base_socket::socket_address_in &address, snl::base_socket::socket_address_len &len) {
			return ::recvfrom(socket, Data, SizePacket, Flags, (struct sockaddr*)&address, &len);
		}
	}
}

// socket network library
namespace snl {

	class client;
	class server;

	typedef void(*function_socket_byte_t)(snl::socket_t, snl::base_socket::byte_t*);
	typedef void(*function_client_byte_t)(snl::client, snl::base_socket::byte_t*);
	typedef void(*function_server_byte_t)(snl::server, snl::base_socket::byte_t*);
	typedef void(*function_server_client_byte_t)(snl::server, snl::client, snl::base_socket::byte_t *);

	typedef void(*function_socket_byte_uint32_t)(snl::socket_t, snl::base_socket::byte_t *, uint32_t);
	typedef void(*function_client_byte_uint32_t)(snl::client, snl::base_socket::byte_t *, uint32_t);
	typedef void(*function_server_byte_uint32_t)(snl::server, snl::base_socket::byte_t *, uint32_t);
	typedef void(*function_server_client_byte_uint32_t)(snl::server, snl::client, snl::base_socket::byte_t *, uint32_t);

	/* Для ООП так как содержит поинты на this. CLOSURE_SUPPORT является расширением поддержки ООП для борланда вплоть до XE10.2. Их фича и не
	является стандартом, но к сожалению необходима.
	*/
	typedef void(CLOSURE_SUPPORT *function_socket_byte_point_t)(void *, snl::socket_t, snl::base_socket::byte_t*);
	typedef void(CLOSURE_SUPPORT *function_client_byte_point_t)(void *, snl::client, snl::base_socket::byte_t*);
	typedef void(CLOSURE_SUPPORT *function_server_byte_point_t)(void *, snl::server, snl::base_socket::byte_t*);
	typedef void(CLOSURE_SUPPORT *function_server_client_byte_point_t)(void *, snl::server, snl::client, snl::base_socket::byte_t *);

	typedef void(CLOSURE_SUPPORT *function_socket_byte_uint32_point_t)(void *, snl::socket_t, snl::base_socket::byte_t *, uint32_t);
	typedef void(CLOSURE_SUPPORT *function_client_byte_uint32_point_t)(void *, snl::client, snl::base_socket::byte_t *, uint32_t);
	typedef void(CLOSURE_SUPPORT *function_server_byte_uint32_point_t)(void *, snl::server, snl::base_socket::byte_t *, uint32_t);
	typedef void(CLOSURE_SUPPORT *function_server_client_byte_uint32_point_t)(void *, snl::server, snl::client, snl::base_socket::byte_t *, uint32_t);

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

		void recreate(const uint32_t _count)
		{
			if (_count == 0)
				return;

			max = _count;

			if (function_obj != nullptr)
				delete[] function_obj;


			function_obj = new table_function_t<T>[max];
		}

		inline  uint32_t GetCount() const  { return count; }
		inline  uint32_t GetMax()   const  { return max;   }

		uint32_t  count = 0;
		uint32_t  max   = 0;

		void AddFunction(const std::string &Name, const T Func) {

			if (GetCount() + 1 > GetMax()) return;

			function_obj[count].Name     = Name;
			function_obj[count].Function = Func;

			count++;
		}

		inline T *GetFunction(const uint32_t &index) const { return index > max ? nullptr : function_obj[index];	}

		T *GetFunction(const std::string &name) const {
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

			function_socket_byte_point.recreate(ReCreateCount);
			function_client_byte_point.recreate(ReCreateCount);
			function_server_byte_point.recreate(ReCreateCount);
			function_server_client_byte_point.recreate(ReCreateCount);
			function_socket_byte_uint32_point.recreate(ReCreateCount);
			function_client_byte_uint32_point.recreate(ReCreateCount);
			function_server_byte_uint32_point.recreate(ReCreateCount);
			function_server_client_byte_uint32_point.recreate(ReCreateCount);

			return *this;
		}

		void RunForBasePacket(void *class_point, std::string Name, snl::socket_t SocketFrom, snl::server server, snl::client client, snl::base_socket::byte_t *Data, uint32_t SizeBuffer);

		inline void Add(const std::string &Name, const  function_socket_byte_t        func)        { function_socket_byte.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_client_byte_t        func)        { function_client_byte.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_server_byte_t        func)        { function_server_byte.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_server_client_byte_t func)        { function_server_client_byte.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_socket_byte_uint32_t func)        { function_socket_byte_uint32.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_client_byte_uint32_t func)        { function_client_byte_uint32.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_server_byte_uint32_t func)        { function_server_byte_uint32.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_server_client_byte_uint32_t func) { function_server_client_byte_uint32.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_socket_byte_point_t         func) { function_socket_byte_point.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_client_byte_point_t         func) { function_client_byte_point.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_server_byte_point_t         func) { function_server_byte_point.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_server_client_byte_point_t  func) { function_server_client_byte_point.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_socket_byte_uint32_point_t  func) { function_socket_byte_uint32_point.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_client_byte_uint32_point_t  func) { function_client_byte_uint32_point.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_server_byte_uint32_point_t  func) { function_server_byte_uint32_point.AddFunction(Name, func); }
		inline void Add(const std::string &Name, const  function_server_client_byte_uint32_point_t func) { function_server_client_byte_uint32_point.AddFunction(Name, func); }

		bool Run(const std::string &Name, snl::socket_t socket, snl::base_socket::byte_t *Buffer);
		bool Run(const std::string &Name, snl::client   client, snl::base_socket::byte_t *Buffer);
		bool Run(const std::string &Name, snl::server   server, snl::base_socket::byte_t *Buffer);
		bool Run(const std::string &Name, snl::socket_t socket, snl::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(const std::string &Name, snl::client   client, snl::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(const std::string &Name, snl::server   server, snl::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(const std::string &Name, snl::server   server, snl::client client, snl::base_socket::byte_t *Buffer);
		bool Run(const std::string &Name, snl::server   server, snl::client client, snl::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(void *point_class, const std::string &Name, snl::socket_t socket, snl::base_socket::byte_t *Buffer);
		bool Run(void *point_class, const std::string &Name, snl::client   client, snl::base_socket::byte_t *Buffer);
		bool Run(void *point_class, const std::string &Name, snl::server   server, snl::base_socket::byte_t *Buffer);
		bool Run(void *point_class, const std::string &Name, snl::socket_t socket, snl::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(void *point_class, const std::string &Name, snl::client   client, snl::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(void *point_class, const std::string &Name, snl::server   server, snl::base_socket::byte_t *Buffer, uint32_t SizePacket);
		bool Run(void *point_class, const std::string &Name, snl::server   server, snl::client client, snl::base_socket::byte_t *Buffer);
		bool Run(void *point_class, const std::string &Name, snl::server   server, snl::client client, snl::base_socket::byte_t *Buffer, uint32_t SizePacket);

		list_table_function_t<function_socket_byte_t>               function_socket_byte = 0;
		list_table_function_t<function_client_byte_t>               function_client_byte = 0;
		list_table_function_t<function_server_byte_t>               function_server_byte = 0;
		list_table_function_t<function_server_client_byte_t>        function_server_client_byte = 0;
		list_table_function_t<function_socket_byte_uint32_t>        function_socket_byte_uint32 = 0;
		list_table_function_t<function_client_byte_uint32_t>        function_client_byte_uint32 = 0;
		list_table_function_t<function_server_byte_uint32_t>        function_server_byte_uint32 = 0;
		list_table_function_t<function_server_client_byte_uint32_t> function_server_client_byte_uint32 = 0;

		list_table_function_t<function_socket_byte_point_t>               function_socket_byte_point = 0;
		list_table_function_t<function_client_byte_point_t>               function_client_byte_point = 0;
		list_table_function_t<function_server_byte_point_t>               function_server_byte_point = 0;
		list_table_function_t<function_server_client_byte_point_t>        function_server_client_byte_point = 0;
		list_table_function_t<function_socket_byte_uint32_point_t>        function_socket_byte_uint32_point = 0;
		list_table_function_t<function_client_byte_uint32_point_t>        function_client_byte_uint32_point = 0;
		list_table_function_t<function_server_byte_uint32_point_t>        function_server_byte_uint32_point = 0;
		list_table_function_t<function_server_client_byte_uint32_point_t> function_server_client_byte_uint32_point = 0;
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
		error_no_send,
		error_size_func_name,
		error_null_send
	};

	enum class arch_server_t
	{
		unknow = 0,
		tcp_thread,               // Каждый клиент имеет свой собственный поток
		udp_thread                // Будет исполненно только одним потоком

	};

	// Архитектуры
	enum class arch_t
	{
		unknow = 0,
		tcp_thread,
		udp_thread
	};

	enum class type_blocked_t {
		non_block,
		block
	};


	class socket_base_t
	{
	public:
		socket_base_t() { TableRunFunction = new TableFunction; *TableRunFunction = 1; }
		socket_base_t(uint32_t CountFunction) { if (CountFunction == 0) return;  TableRunFunction = new TableFunction; *TableRunFunction = CountFunction; }

		~socket_base_t() {
			if (TableRunFunction != nullptr) {
				delete TableRunFunction;
				TableRunFunction = nullptr;
			}
		}

		void ReadPacketTcp(void *point_class, snl::socket_t SocketFrom, socket_base_t *socket_base_client, socket_base_t *socket_base_server);
		void ReadPacketUdp(void *point_class, snl::socket_t SocketFrom, socket_base_t *socket_base_server);
		void RunPacket(void *point_class, std::string Name, snl::socket_t SocketFrom, snl::server server, snl::client client, snl::base_socket::byte_t *Data, uint32_t SizeBuffer);

		inline void close() { isRun = false;  snl::base_socket::close(socket); }
		void InitAddr() { ip = inet_ntoa(address.sin_addr); port = snl::base_socket::ntohs(address.sin_port); lenAddress = sizeof(address); }

		socket_base_t& operator = (const snl::base_socket::socket_address_in _address) { address = _address; InitAddr();  return *this; }
		socket_base_t& operator = (const ipv4 _ip) { ip = _ip; return *this; }
		socket_base_t& operator = (const snl::socket_t _socket) { socket = _socket; return *this; }

		uint32_t SizeRead = 2048;

		bool          isRun = true;
		snl::port_t   port = 0;
		snl::socket_t socket = 0;
		snl::ipv4     ip = "0.0.0.0";	
		snl::base_socket::socket_address_in  address;
		snl::base_socket::socket_address_len lenAddress = 0;
		arch_server_t arch = arch_server_t::unknow;
		TableFunction *TableRunFunction = nullptr;
		snl::function_invoke FunctionInvoke;

		char SelectFunction[32] = { 0 };

		inline void ShowError()	{std::cout << get_error().c_str() << std::endl;	}

		std::string get_error()
		{
			std::string str;
			get_error(str);
			return str;
		}

		void get_error(std::string &str)
		{
			snl::base_socket::byte_t error[32] = { 0 };
			snl::base_socket::socket_len_t len = sizeof(error);
			int retval = snl::base_socket::getsockopt(socket, SOL_SOCKET, SO_ERROR, error, len);

			if (retval != 0) {
				str = strerror(retval);
				return;
			}

			if (error != 0)
				str = strerror(atoi(error));
		}
	};

	class server : public socket_base_t
	{
	public:

		server(const int32_t _SizePacketBuffer = socket_base_mss, const int32_t CountFunction = socket_base_tb, void *point_class = NULL) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction) {
			SetPoint(point_class);
		}

		template<typename TypeFunction_1>
		server(
			const std::string    &Name_1,
			const TypeFunction_1 &Func_1,
			const uint32_t       _SizePacketBuffer = socket_base_mss,
			const uint32_t       CountFunction = socket_base_tb,
			void                *point_class = NULL
		) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction) {
			SetPoint(point_class);
			TableRunFunction->Add(Name_1, Func_1);
		}

		template<typename TypeFunction_1, typename TypeFunction_2>
		server(
			const std::string    &Name_1,
			const TypeFunction_1 &Func_1,
			const std::string    &Name_2,
			const TypeFunction_2 &Func_2,
			const std::size_t    _SizePacketBuffer = socket_base_mss,
			const uint32_t       CountFunction = socket_base_tb,
			void                *point_class = NULL
		) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction)
		{
			SetPoint(point_class);
			TableRunFunction->Add(Name_1, Func_1);
			TableRunFunction->Add(Name_2, Func_2);
		}

		template<typename TypeFunction_1, typename TypeFunction_2, typename TypeFunction_3>
		server(
			const std::string    &Name_1,
			const TypeFunction_1 &Func_1,
			const std::string    &Name_2,
			const TypeFunction_2 &Func_2,
			const std::string    &Name_3,
			const TypeFunction_3 &Func_3,
			const std::size_t    _SizePacketBuffer = socket_base_mss,
			const uint32_t       CountFunction = socket_base_tb,
			void                *point_class = NULL
		) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction)
		{
			SetPoint(point_class);
			TableRunFunction->Add(Name_1, Func_1);
			TableRunFunction->Add(Name_2, Func_2);
			TableRunFunction->Add(Name_3, Func_3);
		}

		template<typename TypeFunction_1, typename TypeFunction_2, typename TypeFunction_3, typename TypeFunction_4>
		server(
			const std::string    &Name_1,
			const TypeFunction_1 &Func_1,
			const std::string    &Name_2,
			const TypeFunction_2 &Func_2,
			const std::string    &Name_3,
			const TypeFunction_3 &Func_3,
			const std::string    &Name_4,
			const TypeFunction_4 &Func_4,
			const std::size_t    _SizePacketBuffer = socket_base_mss,
			const uint32_t       CountFunction = socket_base_tb,
			void                *point_class = NULL
		) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction)
		{
			SetPoint(point_class);
			TableRunFunction->Add(Name_1, Func_1);
			TableRunFunction->Add(Name_2, Func_2);
			TableRunFunction->Add(Name_3, Func_3);
			TableRunFunction->Add(Name_4, Func_4);
		}

		std::size_t SizePacketBuffer = 0;

		server &operator[]  (const char *NameFunction)
		{
			strcpy(SelectFunction, NameFunction);
			return *this;
		}

		void SetPoint(void *newpoint) { point_class = newpoint; }

		void *point_class;

		server& operator = (const function_socket_byte_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_client_byte_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_byte_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_client_byte_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_socket_byte_uint32_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_client_byte_uint32_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_byte_uint32_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_client_byte_uint32_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }

		server& operator = (const function_socket_byte_point_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_client_byte_point_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_byte_point_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_client_byte_point_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_socket_byte_uint32_point_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_client_byte_uint32_point_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_byte_uint32_point_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		server& operator = (const function_server_client_byte_uint32_point_t Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }

		template<typename func_t>
		server& operator = (const func_t &Func) { FunctionInvoke.AddFunction(SelectFunction, Func);	return *this; }

		server& operator = (socket_base_t *SB) {
			
			this->address = SB->address;
			this->arch = SB->arch;
			this->socket = SB->socket;
			this->ip = SB->ip;
			this->socket = SB->socket;
			this->isRun = SB->isRun;
			this->lenAddress = SB->lenAddress;
			this->port = SB->port;
			this->TableRunFunction = SB->TableRunFunction;
			this->FunctionInvoke = SB->FunctionInvoke;
			
			return *this; 
		}

		server& operator = (const socket_base_t &SB) {

			this->address = SB.address;
			this->arch = SB.arch;
			this->socket = SB.socket;
			this->ip = SB.ip;
			this->socket = SB.socket;
			this->isRun = SB.isRun;
			this->lenAddress = SB.lenAddress;
			this->port = SB.port;
			this->TableRunFunction = SB.TableRunFunction;
			this->FunctionInvoke = SB.FunctionInvoke;

			return *this;
		}

		server& operator = (const socket_base_t *SB) {

			this->address = SB->address;
			this->arch = SB->arch;
			this->socket = SB->socket;
			this->ip = SB->ip;
			this->socket = SB->socket;
			this->isRun = SB->isRun;
			this->lenAddress = SB->lenAddress;
			this->port = SB->port;
			this->TableRunFunction = SB->TableRunFunction;
			this->FunctionInvoke = SB->FunctionInvoke;
			return *this;
		}


		status_t create(snl::ipv4 ipServer, snl::port_t portServer, snl::base_socket::address_families family, snl::base_socket::type_protocol type, snl::base_socket::ipproto ipproto)
		{
			lenAddress = sizeof(address);
			socket = snl::base_socket::socket(family, type, ipproto);

			if (socket == socket_error)
				return status_t::error_create_socket;

			snl::base_socket::create_address(address, family, ipServer, portServer);
			int32_t status = snl::base_socket::bind(socket, address);

			if (status == socket_error)
				return status_t::error_bind_socket;


			return status_t::success;
		}

		status_t create(snl::ipv4 ipServer, snl::port_t portServer, arch_server_t archServer, const int32_t &listen = 5)
		{
			if (archServer == arch_server_t::tcp_thread)
			{
				status_t status = create(ipServer, portServer, snl::base_socket::address_families::inet, snl::base_socket::type_protocol::stream, snl::base_socket::ipproto::tcp);
				if (status != status_t::success)
					return status;

				int32_t statuslisten = snl::base_socket::listen(socket, listen);

				if (statuslisten == socket_error)
					return status_t::error_listen_socket;

				arch = archServer;

				return status_t::success;
			}

			if (archServer == arch_server_t::udp_thread)
			{
				status_t status = create(ipServer, portServer, snl::base_socket::address_families::inet, snl::base_socket::type_protocol::dgram, snl::base_socket::ipproto::udp);

				if (status != status_t::success)
					return status;

				arch = archServer;

				return status_t::success;
			}


			return status_t::error_no_select_arch;
		}

		status_t run(type_blocked_t type = type_blocked_t::block)
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
		inline status_t send(snl::socket_t socketClient, snl::base_socket::byte_t *Packet, int32_t SizePacket)	{ snl::base_socket::send(socketClient, Packet, SizePacket, 0);	}
	private:
		status_t RunServer()
		{
			if (socket == socket_error)
				return status_t::error_no_create_socket;

			socket_base_t *server = new socket_base_t(0);

			server->address = this->address;
			server->socket  = this->socket;
			server->arch    = this->arch;
			

			if (arch == arch_server_t::tcp_thread)
			{
				while (true)
				{
					socket_base_t *client = new socket_base_t(0);
					client->lenAddress = sizeof(client->address);
					client->arch = this->arch;

					client->socket = snl::base_socket::accept(socket, client->address, client->lenAddress);

					//	printf("::accept\n");

					if (client->socket == -1)// socket_error)
					{
//						printf("client->socket = %d and it error! %d\n", client->socket, SO_ERROR);
						delete client;
						continue;
					}

					//printf("::Start open new thread &socket_base_t::ReadPacketThreadStream\n");
					std::thread   threadclient(&socket_base_t::ReadPacketTcp, this, this->point_class, client->socket, client, server);
					threadclient.detach();
				}

				delete server;
				return status_t::success;
			}

			if (arch == arch_server_t::udp_thread)
				ReadPacketUdp(this->point_class, socket, server);

			delete server;
			return status_t::error_no_select_arch;
		}
	};

	class client : public socket_base_t
	{
	public:

		/*client() : SizePacketBuffer(1440), socket_base_t(32) {
		CreatePacketBuffer(SizePacketBuffer);
		}
		client(std::size_t _SizePacketBuffer, uint32_t CountFunction = 32) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction) {

		}*/

		client(const std::size_t _SizePacketBuffer = socket_base_mss, const int32_t CountFunction = socket_base_tb, void *point_class = NULL) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction) { SetPoint(point_class); CreatePacketBuffer(SizePacketBuffer); }

		template<typename TypeFunction_1>
		client(
			const std::string    &Name_1,
			const TypeFunction_1 &Func_1,
			const uint32_t       _SizePacketBuffer = socket_base_mss,
			const uint32_t       CountFunction = socket_base_tb,
			void                *point_class = NULL
		) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction) {
			SetPoint(point_class);
			TableRunFunction->Add(Name_1, Func_1);
			CreatePacketBuffer(SizePacketBuffer);
		}

		template<typename TypeFunction_1, typename TypeFunction_2>
		client(
			const std::string    &Name_1,
			const TypeFunction_1 &Func_1,
			const std::string    &Name_2,
			const TypeFunction_2 &Func_2,
			const std::size_t    _SizePacketBuffer = socket_base_mss,
			const uint32_t       CountFunction = socket_base_tb,
			void                *point_class = NULL
		) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction)
		{
			SetPoint(point_class);
			TableRunFunction->Add(Name_1, Func_1);
			TableRunFunction->Add(Name_2, Func_2);
			CreatePacketBuffer(SizePacketBuffer);
		}

		template<typename TypeFunction_1, typename TypeFunction_2, typename TypeFunction_3>
		client(
			const std::string    &Name_1,
			const TypeFunction_1 &Func_1,
			const std::string    &Name_2,
			const TypeFunction_2 &Func_2,
			const std::string    &Name_3,
			const TypeFunction_3 &Func_3,
			const std::size_t    _SizePacketBuffer = socket_base_mss,
			const uint32_t       CountFunction = socket_base_tb,
			void                *point_class = NULL
		) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction)
		{
			SetPoint(point_class);
			TableRunFunction->Add(Name_1, Func_1);
			TableRunFunction->Add(Name_2, Func_2);
			TableRunFunction->Add(Name_3, Func_3);
			CreatePacketBuffer(SizePacketBuffer);
		}

		template<typename TypeFunction_1, typename TypeFunction_2, typename TypeFunction_3, typename TypeFunction_4>
		client(
			const std::string    &Name_1,
			const TypeFunction_1 &Func_1,
			const std::string    &Name_2,
			const TypeFunction_2 &Func_2,
			const std::string    &Name_3,
			const TypeFunction_3 &Func_3,
			const std::string    &Name_4,
			const TypeFunction_4 &Func_4,
			const std::size_t    _SizePacketBuffer = socket_base_mss,
			const uint32_t       CountFunction = socket_base_tb,
			void                *point_class = NULL
		) : SizePacketBuffer(_SizePacketBuffer), socket_base_t(CountFunction)
		{
			SetPoint(point_class);
			TableRunFunction->Add(Name_1, Func_1);
			TableRunFunction->Add(Name_2, Func_2);
			TableRunFunction->Add(Name_3, Func_3);
			TableRunFunction->Add(Name_4, Func_4);
			CreatePacketBuffer(SizePacketBuffer);
		}

		status_t connect(snl::ipv4 ipServer, snl::port_t portServer, snl::base_socket::address_families family, snl::base_socket::type_protocol type, snl::base_socket::ipproto ipproto)
		{
			lenAddress = sizeof(address);
			socket = snl::base_socket::socket(family, type, ipproto);

			if (socket == socket_error)
				return status_t::error_create_socket;

			snl::base_socket::create_address(address, family, ipServer, portServer);
			int32_t
				status = snl::base_socket::connect(socket, address, lenAddress);

			if (status == socket_error)
				return status_t::error_connect;

			return status_t::success;
		}
		status_t connect(snl::ipv4 ipServer, snl::port_t portServer, arch_server_t archServer)
		{
			if (archServer == arch_server_t::tcp_thread)
			{
				status_t status = connect(ipServer, portServer, snl::base_socket::address_families::inet, snl::base_socket::type_protocol::stream, snl::base_socket::ipproto::tcp);

				if (status != status_t::success)
					return status;

				arch = archServer;

				return status_t::success;
			}

			if (archServer == arch_server_t::udp_thread)
			{
				lenAddress = sizeof(address);
				socket = snl::base_socket::socket(snl::base_socket::address_families::inet, snl::base_socket::type_protocol::dgram, snl::base_socket::ipproto::udp);

				if (socket == socket_error)
					return status_t::error_create_socket;

				snl::base_socket::create_address(address, snl::base_socket::address_families::inet, ipServer, portServer);

				arch = archServer;

				return status_t::success;
			}

			return status_t::error_no_select_arch;
		}
		status_t run(type_blocked_t type = type_blocked_t::block)
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


		std::size_t SizePacketBuffer;

		void SetPoint(void *newpoint) { point_class = newpoint; }

		void *point_class;

		template<typename T>
		inline status_t base_send(const T *objSend, std::size_t Size)	{	return this->send(static_cast<snl::base_socket::byte_t*>(objSend), Size);		}

		template<typename ..._Args>
		status_t send(const char *NameFunction, _Args && ... args)
		{
			snl::stream_buffer socket_buffer;
			socket_buffer.push_back(args...);
	
			int32_t size = 0;	
			for (auto &it : socket_buffer)
				size += it.size();

			PacketBuffer[0] = static_cast<char>(snl::packet_header_t::wrapertable_arg_packet);

			if (strlen(NameFunction) > 32)
				return snl::status_t::error_size_func_name;

			memcopy(PacketBuffer + 1, NameFunction, 32);
			
			socket_buffer.data_allbuffer(PacketBuffer + 1 + 32);
			int32_t total = sizeof(snl::base_socket::byte_t) + 32 /*Size function name*/ + size + (socket_buffer.size() * sizeof(int32_t)) + sizeof(int32_t);
			return base_send(PacketBuffer, total);
		}

		status_t base_send(const snl::base_socket::byte_t *Packet, const int32_t &SizePacket)
		{
			if (arch == arch_server_t::tcp_thread)		
				if (snl::base_socket::send(socket, Packet, SizePacket, 0) == socket_error)
					return status_t::error_no_send;
			
			if (arch == arch_server_t::udp_thread)			
				if (snl::base_socket::send(socket, Packet, SizePacket, 0, address, lenAddress) == socket_error)
				    return status_t::error_no_send;
			
			return status_t::success;
		}



		client &operator[]  (const char *NameFunction)
		{
			strcpy(SelectFunction, NameFunction);
			return *this;
		}

		inline client& operator = (const function_socket_byte_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_client_byte_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_server_byte_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_server_client_byte_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_socket_byte_uint32_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_client_byte_uint32_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_server_byte_uint32_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_server_client_byte_uint32_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }

		inline client& operator = (const function_socket_byte_point_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_client_byte_point_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_server_byte_point_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_server_client_byte_point_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_socket_byte_uint32_point_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_client_byte_uint32_point_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_server_byte_uint32_point_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }
		inline client& operator = (const function_server_client_byte_uint32_point_t &Func) { TableRunFunction->Add(SelectFunction, Func);	return *this; }

		template<typename T>
		inline client& operator = (const T *data) { return *this = *data; }

		client& operator = (const snl::socket_base_t &socket_base) {

		   this->address = socket_base.address;
		   this->arch    = socket_base.arch;
		   this->socket  = socket_base.socket;
		   this->ip      = socket_base.ip;
		   this->socket  = socket_base.socket;
		   this->isRun   = socket_base.isRun;
		   this->port    = socket_base.port;
		   this->lenAddress = socket_base.lenAddress;
		   this->TableRunFunction = socket_base.TableRunFunction;

			return *this;
		}


		status_t wraper_send(const std::string &func_name, const snl::base_socket::byte_t *Packet, int32_t SizePacket)
		{
			PacketBuffer[0] = static_cast<char>(snl::packet_header_t::wrapertable_packet);

			if (func_name.length() > 32 || func_name.empty())
				return snl::status_t::error_size_func_name;

			memcopy(PacketBuffer+1, func_name.c_str(), 32);
			memcopy(PacketBuffer+1+32, Packet, SizePacket);

			return base_send(PacketBuffer, 1 + 32 + SizePacket);
		}


	private:
		status_t RunClient()
		{
			if (socket == socket_error)
				return status_t::error_no_create_socket;

			if (arch == arch_server_t::tcp_thread)
			{
				// Стоит учесть что 0, это количество аллоциеруемых таблиц
				socket_base_t *client = new socket_base_t(0);
				socket_base_t *server = new socket_base_t(0);

				client->address = this->address;
				server->address = this->address;

				client->socket  = this->socket;
				client->arch    = arch;
				server->arch    = arch;

				std::thread threadclient(&socket_base_t::ReadPacketTcp, this, this->point_class, socket, client, server);
				threadclient.join();

				return status_t::success;
			}

			return status_t::error_no_select_arch;
		}
		snl::base_socket::byte_t *PacketBuffer = nullptr;

		void CreatePacketBuffer(const std::size_t &Size) {
			if (PacketBuffer == nullptr && Size > 0)
				PacketBuffer = new snl::base_socket::byte_t[Size];
		}
		void DeletePacketBuffer() {
			if (PacketBuffer != nullptr) {
				delete[] PacketBuffer;
				PacketBuffer = nullptr;
			}
		}
	};


	bool TableFunction::Run(const std::string &Name, snl::socket_t socket, snl::base_socket::byte_t *Buffer)
	{
		function_socket_byte_t *Func = function_socket_byte.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(socket, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(const std::string &Name, snl::client client, snl::base_socket::byte_t *Buffer)
	{
		function_client_byte_t *Func = function_client_byte.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(client, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(const std::string &Name, snl::server   server, snl::base_socket::byte_t *Buffer)
	{
		function_server_byte_t *Func = function_server_byte.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(server, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(const std::string &Name, snl::server   server, snl::client client, snl::base_socket::byte_t *Buffer)
	{
		function_server_client_byte_t *Func = function_server_client_byte.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(server, client, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(const std::string &Name, snl::socket_t socket, snl::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_socket_byte_uint32_t *Func = function_socket_byte_uint32.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(socket, Buffer, SizePacket));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(const std::string &Name, snl::client   client, snl::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_client_byte_uint32_t *Func = function_client_byte_uint32.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(client, Buffer, SizePacket));
			return true;
		}
		return false;
	}

	bool TableFunction::Run(const std::string &Name, snl::server   server, snl::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_server_byte_uint32_t *Func = function_server_byte_uint32.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(server, Buffer, SizePacket));

			return true;
		}

		return false;
	}

	bool TableFunction::Run(const std::string &Name, snl::server   server, snl::client client, snl::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_server_client_byte_uint32_t *Func = function_server_client_byte_uint32.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(server, client, Buffer, SizePacket));
			return true;
		}
		return false;
	}




	///////////////////////////////////////////////////////////////////////////////

	bool TableFunction::Run(void *point_class, const std::string &Name, snl::socket_t socket, snl::base_socket::byte_t *Buffer)
	{
		function_socket_byte_point_t *Func = function_socket_byte_point.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(point_class, socket, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(void *point_class, const std::string &Name, snl::client client, snl::base_socket::byte_t *Buffer)
	{
		function_client_byte_point_t *Func = function_client_byte_point.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(point_class, client, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(void *point_class, const  std::string &Name, snl::server   server, snl::base_socket::byte_t *Buffer)
	{
		function_server_byte_point_t *Func = function_server_byte_point.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(point_class, server, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(void *point_class, const  std::string &Name, snl::server   server, snl::client client, snl::base_socket::byte_t *Buffer)
	{
		function_server_client_byte_point_t *Func = function_server_client_byte_point.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(point_class, server, client, Buffer));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(void *point_class, const  std::string &Name, snl::socket_t socket, snl::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_socket_byte_uint32_point_t *Func = function_socket_byte_uint32_point.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(point_class, socket, Buffer, SizePacket));
			return true;
		}

		return false;
	}

	bool TableFunction::Run(void *point_class, const  std::string &Name, snl::client   client, snl::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_client_byte_uint32_point_t *Func = function_client_byte_uint32_point.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(point_class, client, Buffer, SizePacket));
			return true;
		}
		return false;
	}

	bool TableFunction::Run(void *point_class, const  std::string &Name, snl::server   server, snl::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_server_byte_uint32_point_t *Func = function_server_byte_uint32_point.GetFunction(Name);

		if (Func != nullptr) {
			((*Func)(point_class, server, Buffer, SizePacket));

			return true;
		}

		return false;
	}

	bool TableFunction::Run(void *point_class, const std::string &Name, const snl::server   server, snl::client client, snl::base_socket::byte_t *Buffer, uint32_t SizePacket)
	{
		function_server_client_byte_uint32_point_t *Func = function_server_client_byte_uint32_point.GetFunction(Name);

		if (Func != nullptr)
		{
			((*Func)(point_class, server, client, Buffer, SizePacket));
			return true;
		}
		return false;
	}

	void socket_base_t::ReadPacketTcp(void *point_class, snl::socket_t SocketFrom, socket_base_t *socket_base, socket_base_t *socket_base_server)
	{
		if (socket_base == nullptr || socket_base_server == nullptr || SocketFrom == -1)
		{
			//	printf("::No open ReadPacketThreadStream\n");
			return;
		}

		//printf("::ReadPacketThreadStream\n");

		socket_base->InitAddr();
		socket_base_server->InitAddr();

		snl::client client(1440, 0);
		snl::server server(0, 0);

		client = socket_base;
		server = socket_base_server;

		if (client.socket == socket_error)
			return;

		if (SizeRead == 0)
			return;

		snl::base_socket::byte_t *Data = new snl::base_socket::byte_t[SizeRead];

		if (Data == nullptr) {
			printf("\nIn socket library cannot alloc memory for new client.");
			snl::base_socket::close(SocketFrom);
			return;
		}

		TableRunFunction->RunForBasePacket(point_class, "new", SocketFrom, server, client, nullptr, 0);

		while (true) {
			int32_t StatusPacket = snl::base_socket::recv(SocketFrom, Data, SizeRead, 0);

			if (StatusPacket > 0) {

				TableRunFunction->RunForBasePacket(point_class, "read", SocketFrom, server, client, Data, StatusPacket);

				if (Data[0] == static_cast<byte_t>(snl::packet_header_t::wrapertable_packet))
				{
					char name_function[32];
					snl::memcopy(name_function, Data+1, 32);
					TableRunFunction->RunForBasePacket(point_class, name_function, SocketFrom, server, client, Data + 32 + 1, StatusPacket-1-32);
				}

				if (Data[0] == static_cast<byte_t>(snl::packet_header_t::wrapertable_arg_packet))
				{
					char name_function[32];
					snl::memcopy(name_function, Data + 1, 32);
				
					snl::stream_buffer arg;
					arg.data_allbuffer_read(Data + 1 + 32);
						
					switch (arg.size()) {
					case 1:
						FunctionInvoke.RunFunction(name_function, arg[0]);  break;
					case 2:
						FunctionInvoke.RunFunction(name_function, arg[1], arg[0]);  break;
					case 3:
						FunctionInvoke.RunFunction(name_function, arg[2], arg[1], arg[0]); break;
					case 4:
						FunctionInvoke.RunFunction(name_function, arg[3], arg[2], arg[1], arg[0]); break;
					case 5:
						FunctionInvoke.RunFunction(name_function, arg[4], arg[3], arg[2], arg[1], arg[0]); break;
					case 6:
						FunctionInvoke.RunFunction(name_function, arg[5], arg[4], arg[3], arg[2], arg[1], arg[0]);  break;
					case 7:
						FunctionInvoke.RunFunction(name_function, arg[6], arg[5], arg[4], arg[3], arg[2], arg[1], arg[0]);  break;
					case 8:
						FunctionInvoke.RunFunction(name_function, arg[7], arg[6], arg[5], arg[4], arg[3], arg[2], arg[1], arg[0]);  break;
					case 9:
						FunctionInvoke.RunFunction(name_function, arg[8], arg[7], arg[6], arg[5], arg[4], arg[3], arg[2], arg[1], arg[0]);  break;
					default:
						break;
					}
				}
			}

			if (!isRun)
			{
				TableRunFunction->RunForBasePacket(point_class, "closeserver", SocketFrom, server, client, nullptr, 0);
				break;
			}

			if (StatusPacket < 0)
			{
				TableRunFunction->RunForBasePacket(point_class, "end", SocketFrom, server, client, nullptr, 0);
				break;
			}
		}

		delete socket_base;
		socket_base = nullptr;

		delete[] Data;
		Data = nullptr;
	}

	void socket_base_t::ReadPacketUdp(void *point_class, snl::socket_t SocketFrom, socket_base_t *socket_base_server)
	{
		if (socket_base_server == nullptr || SocketFrom == -1)
			return;

		socket_base_server->InitAddr();

		socket_base_t socket_base_client(0);
		snl::server server(0, 0);
		snl::client client(0, 0);


		server = socket_base_server;

		if (SizeRead == 0)
			return;

		snl::base_socket::byte_t *Data = new snl::base_socket::byte_t[SizeRead];

		if (Data == nullptr) {
			printf("\nIn socket library cannot alloc memory for new client.");
			snl::base_socket::close(SocketFrom);
			return;
		}

		while (true) {

			socket_base_client.lenAddress = sizeof(socket_base_client.address);
			int32_t StatusPacket = snl::base_socket::recvfrom(SocketFrom, Data, SizeRead, 0, socket_base_client.address, socket_base_client.lenAddress);

			if (StatusPacket < 1)
				continue;

			socket_base_client.InitAddr();

			client = socket_base_client;

			if (StatusPacket > 0) {

				TableRunFunction->RunForBasePacket(point_class, "read", SocketFrom, server, client, Data, StatusPacket);

				if (Data[0] == static_cast<byte_t>(snl::packet_header_t::wrapertable_packet))
				{
					char name_function[32];
					snl::memcopy(name_function, Data+1, 32);
					TableRunFunction->RunForBasePacket(point_class, name_function, SocketFrom, server, client, Data+32+1, StatusPacket);
				}

			}

			if (!isRun) {
				TableRunFunction->RunForBasePacket(point_class, "closeserver", SocketFrom, server, client, nullptr, 0);
				break;
			}

			if (StatusPacket < 0)
				break;
		}

		delete[] Data;
		Data = nullptr;
	}


	void TableFunction::RunForBasePacket(void *class_point, std::string Name, snl::socket_t SocketFrom, snl::server server, snl::client client, snl::base_socket::byte_t *Data, uint32_t SizeBuffer)
	{
		if (class_point == NULL) {
			if (!Run(Name, SocketFrom, Data))
				if (!Run(Name, SocketFrom, Data, SizeBuffer))
					if (!Run(Name, client, Data))
						if (!Run(Name, client, Data, SizeBuffer))
							if (!Run(Name, server, Data))
								if (!Run(Name, server, Data, SizeBuffer))
									if (!Run(Name, server, client, Data))
										Run(Name, server, client, Data, SizeBuffer);
		}
		else
		{
			if (!Run(class_point, Name, SocketFrom, Data))
				if (!Run(class_point, Name, SocketFrom, Data, SizeBuffer))
					if (!Run(class_point, Name, client, Data))
						if (!Run(class_point, Name, client, Data, SizeBuffer))
							if (!Run(class_point, Name, server, Data))
								if (!Run(class_point, Name, server, Data, SizeBuffer))
									if (!Run(class_point, Name, server, client, Data))
										Run(class_point, Name, server, client, Data, SizeBuffer);
		}
	}

	void socket_base_t::RunPacket(void *point_class, std::string Name, snl::socket_t SocketFrom, snl::server server, snl::client client, snl::base_socket::byte_t *Data, uint32_t SizeBuffer)
	{
		if (server.arch == arch_server_t::tcp_thread || server.arch == arch_server_t::udp_thread)
			TableRunFunction->RunForBasePacket(point_class, Name, SocketFrom, server, client, Data, SizeBuffer);
	}


	struct ObjPacket {
		ObjPacket() : server(0, 0), client(0, 0) {
			Size = 0;
			Packet = 0;
		}

		uint32_t Size;
		snl::base_socket::byte_t *Packet;
		snl::server server;
		snl::client client;
		std::string Name = "";
		snl::socket_t SocketFrom = 0;
	};

	class socket_queue
	{
	public:
		socket_queue();
		~socket_queue();

		snl::socket_t       Socket;

		bool EndData();
		bool CreateListPacket(uint32_t Size);
		bool ReCreatePacketList(uint32_t Size);
		bool AddPacket(std::string Name, snl::socket_t SocketFrom, snl::server server, snl::client client, snl::base_socket::byte_t *Data, uint32_t SizeBuffer);
		bool GetPacket(std::string &Name, snl::socket_t &SocketFrom, snl::server &server, snl::client &client, snl::base_socket::byte_t *Data, uint32_t &SizeBuffer);

		void RunAllPacket();

		uint32_t GetSizeLastBuffer();

		TableFunction *TableRunFunction;

	private:
		uint32_t      SizeList, LastIndex;
		ObjPacket    *ListPacket;
		bool          isCreateListPacket;
		uint32_t      MemoryCopy(char *Buffer, char* Obj, uint32_t PositionBuffer, uint32_t StartReadPosition, uint32_t EndReadPosition);
		void          RunPacket(std::string Name, snl::socket_t SocketFrom, snl::server server, snl::client client, snl::base_socket::byte_t *Data, uint32_t SizeBuffer);
	};



	socket_queue::socket_queue() {
		SizeList = 0;
		LastIndex = 0;
		isCreateListPacket = false;
	}

	socket_queue::~socket_queue()
	{
		if (isCreateListPacket) {
			delete[] ListPacket;
			ListPacket = NULL;
		}
	}


	bool socket_queue::EndData()
	{
		if (LastIndex != 0)
			return true;

		return false;
	}

	bool socket_queue::CreateListPacket(uint32_t Size) {

		if (isCreateListPacket == true)
			return false;

		isCreateListPacket = true;

		ListPacket = new ObjPacket[Size];
		SizeList = Size;

		return true;
	}

	bool socket_queue::ReCreatePacketList(uint32_t Size) {

		if (isCreateListPacket)
		{
			delete[] ListPacket;
			ListPacket = NULL;
		}

		SizeList = 0;
		LastIndex = 0;
		isCreateListPacket = false;

		CreateListPacket(Size);
		return true;
	}

	bool socket_queue::AddPacket(std::string Name, snl::socket_t SocketFrom, snl::server server, snl::client client, snl::base_socket::byte_t *Data, uint32_t SizeBuffer) {

		if (Data == NULL || SizeBuffer == 0 || (LastIndex + 1) > SizeList)
			return false;

		std::mutex Mutex;

		Mutex.lock();
		LastIndex++;

		ListPacket[LastIndex].Packet = new char[SizeBuffer];
		ListPacket[LastIndex].Size = SizeBuffer;
		MemoryCopy(ListPacket[LastIndex].Packet, Data, 0, 0, SizeBuffer);

		ListPacket[LastIndex].Name = Name;
		ListPacket[LastIndex].SocketFrom = SocketFrom;
		ListPacket[LastIndex].server = server;
		ListPacket[LastIndex].client = client;


		Mutex.unlock();

		return true;
	}

	uint32_t socket_queue::GetSizeLastBuffer() {

		if (LastIndex == 0)
			return 0;

		return ListPacket[LastIndex].Size;
	}


	bool socket_queue::GetPacket(std::string &Name, snl::socket_t &SocketFrom, snl::server &server, snl::client &client, snl::base_socket::byte_t *Data, uint32_t &SizeBuffer) {

		if (Data == NULL || LastIndex == 0 || ListPacket[LastIndex].Packet == NULL || GetSizeLastBuffer() == 0)
			return false;

		std::mutex  Mutex;
		Mutex.lock();

		MemoryCopy(Data, ListPacket[LastIndex].Packet, 0, 0, ListPacket[LastIndex].Size);
		SizeBuffer = ListPacket[LastIndex].Size;

		delete[] ListPacket[LastIndex].Packet;

		ListPacket[LastIndex].Packet = NULL;
		ListPacket[LastIndex].Size = 0;

		SocketFrom = ListPacket[LastIndex].SocketFrom;
		server = ListPacket[LastIndex].server;
		client = ListPacket[LastIndex].client;
		Name = ListPacket[LastIndex].Name;

		LastIndex--;

		Mutex.unlock();

		return true;
	}

	uint32_t socket_queue::MemoryCopy(char *Buffer, char* Obj, uint32_t PositionBuffer, uint32_t StartReadPosition, uint32_t EndReadPosition)
	{

		uint32_t w = PositionBuffer;
		for (uint32_t i = StartReadPosition; i < EndReadPosition; i++)
		{
			Buffer[w] = Obj[i];
			w++;
		}

		return w;
	}


/*	void socket_queue::RunPacket(std::string Name, snl::socket_t SocketFrom, snl::server server, snl::client client, snl::base_socket::byte_t *Data, uint32_t SizeBuffer)
	{

	}*/

	void socket_queue::RunAllPacket() {

		while (this->EndData())
		{
			char *BufferPacket = new char[GetSizeLastBuffer()];

			//uint32_t Size;
			//GetPacket(BufferPacket, Size);
			//RunPacket(BufferPacket, Socket, ServerClient);
			delete[] BufferPacket;
		}
	}

}
