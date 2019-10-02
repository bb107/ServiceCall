#pragma once
#include <string>
#include <vector>

#ifndef BSAPI
#define BSAPI __stdcall
#endif
#define CP_ACP                    0           // default to ANSI code page
typedef const char *LPCSTR, *PCSTR;
#define IN
#ifdef _WIN64
#pragma warning(disable:4267)
#endif

std::wstring BSAPI CharToWchar(const char* c, size_t m_encode = CP_ACP);
std::string BSAPI WCharToChar(const wchar_t* c, size_t m_encode = CP_ACP);
