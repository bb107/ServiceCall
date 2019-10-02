#include "str.h"
#include <Windows.h>

std::wstring BSAPI CharToWchar(const char* c, size_t m_encode) {
	std::wstring str = L"";	if (c == NULL || *c == NULL)return str;
	int len = MultiByteToWideChar(m_encode, 0, c, strlen(c), NULL, 0); wchar_t* m_wchar = new wchar_t[len + 1];
	MultiByteToWideChar(m_encode, 0, c, strlen(c), m_wchar, len); m_wchar[len] = '\0';
	str = m_wchar; delete[] m_wchar; return str;
}

std::string BSAPI WCharToChar(const wchar_t* c, size_t m_encode) {
	std::string str = "";	if (c == NULL || *c == NULL)return str;
	int len = WideCharToMultiByte(m_encode, 0, c, -1, NULL, 0, NULL, NULL); char* m_char = new char[len + 1];
	WideCharToMultiByte(m_encode, 0, c, -1, m_char, len, NULL, NULL);
	str = m_char; delete[] m_char; return str;
}
