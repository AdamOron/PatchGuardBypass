#pragma once
#include <ntdef.h>

template <typename T, SIZE_T Capacity>
class FixedArray
{
private:
	T m_Elements[Capacity];
	SIZE_T m_Size;

public:
	FixedArray() :
		m_Elements(),
		m_Size(0)
	{
	}

	T &Get(SIZE_T Index)
	{
		return m_Elements[Index];
	}

	bool Append(T &Element)
	{
		if (m_Size >= Capacity)
			return false;

		m_Elements[m_Size++] = Element;
		return true;
	}

	bool Append(T Element)
	{
		if (m_Size >= Capacity)
			return false;

		m_Elements[m_Size++] = Element;
		return true;
	}

	SIZE_T Size() const
	{
		return m_Size;
	}
};

