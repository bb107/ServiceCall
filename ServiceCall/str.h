#pragma once
#include <string>
#include <vector>

#ifdef _EXPORT
#define BSEXPORT __declspec(dllexport)
#define BSAPI __stdcall
#else
#define BSEXPORT
#ifndef BSAPI
#define BSAPI __stdcall
#endif
#endif
#define CP_ACP                    0           // default to ANSI code page
typedef const char *LPCSTR, *PCSTR;
#define IN
#ifdef _WIN64
#pragma warning(disable:4267)
#endif

namespace Boringsoft {
	namespace str {
		const std::string BSAPI Strupr(IN LPCSTR buf);
#define strupr Strupr

		size_t BSAPI RtlStringSplit(const char*  String, const char*  use_for_split, std::vector<std::string>&ret_split);
		size_t BSAPI RtlStringSplit(const std::string String, const std::string use_for_split, std::vector<std::string>&ret_split);

		size_t BSAPI RtlFindString(const char*source, const char*use_for_find);

		std::wstring BSAPI CharToWchar(const char* c, size_t m_encode = CP_ACP);
		std::string BSAPI WCharToChar(const wchar_t* c, size_t m_encode = CP_ACP);

		bool BSAPI RtlEqualString(LPCSTR _left, LPCSTR _right);
	}
}
