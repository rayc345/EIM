#pragma once
#include <string>

std::wstring CharToWchar(const char* c, size_t m_encode = CP_ACP);
std::string WcharToChar(const wchar_t* wp, size_t m_encode = CP_ACP);

std::string base64_encode(LPCSTR s, int len);
std::string base64_decode(std::string const& s);