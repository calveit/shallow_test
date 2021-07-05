#pragma once
#include <atomic>

#include "constants.h"
#include "vector2.h"

typedef int tCountryIndex;
typedef int tProvinceIndex;
typedef int tArmyIndex;

template <typename T>
struct AtomWrapper
{
	std::atomic<T> _a;

	AtomWrapper()
		:_a()
	{}

	AtomWrapper(const std::atomic<T>& a)
		:_a(a.load())
	{}

	AtomWrapper(const AtomWrapper& other)
		:_a(other._a.load())
	{}

	AtomWrapper& operator=(const AtomWrapper& other)
	{
		_a.store(other._a.load());
		return *this;
	}

	AtomWrapper& operator++()
	{
		++_a;
		return *this;
	}
};


struct Country
{
	short provinceStartIndex;
	short provinceCount;
	int armyStartIndex;
	AtomWrapper<int> armyCount;
	float spawnFactor = 1.0f;
	ShallowTest::Vector2 position;
};

struct Province
{
	short prevCountryIndex = -1;
	short countryIndex = -1;
	int armyStartIndex = 0;
	AtomWrapper<int> armyCount = 0;
};

struct Army
{
	ShallowTest::Vector2 position{ 100, 100 };

	// 0-3 Country Index
	// 4 Valid
	// 8-15 Hit Points
	int flags = 0x000000E0;
	int provinceIndex = -1;

	bool isValid() const
	{
		return (flags & 0x00000010) == 0x00000010;
	}

	char getCountryIndex() const
	{
		return (flags & 0x0000000F);
	}

	void setCountryIndex(char index)
	{
		flags = (flags & ~0x0F) | index;
	}

	void invalidate()
	{
		flags &= ~0x00000010;
	}

	void validate()
	{
		flags |= 0x00000010;
	}

	char getHitPoints() { return (flags & 0x0000FF00) >> 8; }
	void setHitPoints(unsigned char hitPoints) { flags = (flags & ~0x0000FF00) | hitPoints << 8; }
};
