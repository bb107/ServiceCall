#include "str.h"
#include <Windows.h>

namespace Boringsoft {
	namespace str {
		const std::string BSAPI Strupr(IN LPCSTR buf) {
			size_t len = strlen(buf) + 1;
			std::string tmp(len, 0);
			for (size_t i = 0; i < len; i++)
				tmp[i] = buf[i] >= 'a'&&buf[i] <= 'z' ? buf[i] - 0x20 : buf[i];
			return tmp;
		}

		size_t BSAPI RtlStringSplit(const char*  String, const char*  use_for_split, std::vector<std::string>&ret_split) {
			if (!String || !use_for_split)return 0;
			size_t len = 0; while (*(String + len))len++;
			size_t step = strlen(use_for_split); char*t_string = new char[len + 1];
			for (size_t i = 0, j = 0; i < len; i++, j++) {
				if (strncmp(String + i, use_for_split, step)) {
					*(t_string + j) = *(String + i); if (i + 1 != len)continue; else j++;
				}
				*(t_string + j) = '\0';  if (*(t_string)) ret_split.push_back(t_string); j = -1; i += step - 1;
			}
			return ret_split.size();
		}

		size_t BSAPI RtlStringSplit(const std::string String, const std::string use_for_split, std::vector<std::string>&ret_split) {
			return RtlStringSplit(String.c_str(), use_for_split.c_str(), ret_split);
		}

		size_t BSAPI RtlFindString(const char*source, const char*use_for_find) {
			size_t len = 0; while (*(source + len))len++; size_t step = strlen(use_for_find);
			for (size_t i = 0; i < len; i++) if (!strncmp(source + i, use_for_find, step))return i;
			return -1;
		}

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

		bool BSAPI RtlEqualString(LPCSTR _left, LPCSTR _right) {
			return !_stricmp(_left, _right);
		}
	}
}
