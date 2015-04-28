#ifndef __SYS_INC_HPP__
#define __SYS_INC_HPP__

#include "sysinc.h"

#include <vector>
#include <list>
#include <queue>
#include <deque>
#include <map>
#include <set>
#include <stack>
#include <algorithm>
#include <utility>
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <exception>

using namespace std;

template<typename T> inline bool contain_item(const std::list<T>* Alist, const T& Aitem)
{
	return std::find(Alist->begin(), Alist->end(), Aitem) != Alist->end();
}

// ---- guid_t c++ operator overloading

inline bool operator ==(const guid_t& Aleft, const guid_t& Aright)
{
	return memcmp(&Aleft, &Aright, sizeof(guid_t)) == 0;
}

inline bool operator !=(const guid_t& Aleft, const guid_t& Aright)
{
	return !(Aleft == Aright);
}

inline bool operator >(const guid_t& Aleft, const guid_t& Aright)
{
	for (int i = 0; i < 16; i++) {
		if (Aleft.bytes[i] < Aright.bytes[i])
			return false;
		if (Aleft.bytes[i] > Aright.bytes[i])
			return true;
	}
	return false;
}

inline bool operator <(const guid_t& Aleft, const guid_t& Aright)
{
	for (int i = 0; i < 16; i++) {
		if (Aleft.bytes[i] > Aright.bytes[i])
			return false;
		if (Aleft.bytes[i] < Aright.bytes[i])
			return true;
	}
	return false;
}

template<typename T> inline bool Pertains(T A, int Acount, ...)
{
	va_list ap;
	va_start(ap, Acount);
	for (int i = 0; i < Acount; i++) {
		if (A == va_arg(ap, T)) {
			va_end(ap);
			return true;
		}
	}
	va_end(ap);
	return false;
}

struct DynBlock : public std::vector<char>
{
	void assign(const void* Abuf, size_t Alen)
	{
		resize(Alen);
		if (Alen > 0)
			::memcpy(&(*this)[0], Abuf, Alen);
	}
	void assign(const std::string& Astr, int Aoff = 0, ssize_t Alen = - 1)
	{
		ssize_t len = Alen < 0 ? Astr.length() : Alen;
		resize(len);
		Astr.copy(&(*this)[0], Alen < 0 ? Astr.length() : Alen, Aoff);
	}
};

#endif // __SYS_INC_HPP__
