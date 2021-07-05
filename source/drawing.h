#pragma once
#include <functional>
#include <vector>

#include "constants.h"
#include "game_state.h"
#include "raylib.h"


template<class T>
using ref = std::reference_wrapper<T>;

struct GameState
{
	ref<const std::vector<tArmyIndex>> armyIndices;
	ref<const std::vector<Army>> armies;

	ref<const std::vector<tCountryIndex>> countryIndices;
	ref<const std::vector<Country>> countries;
	ref<const std::vector<Color>> countryColors;

	ref<const std::vector<tProvinceIndex>> provinceIndices;
	ref<const std::vector<Province>> provinces;
	ref<const std::vector<ShallowTest::Vector2>> flow;
};

struct DrawingContext
{
	std::vector<tArmyIndex> armyIndices;
	std::vector<Army> armies;

	std::vector<tCountryIndex> countryIndices;
	std::vector<Country> countries;

	std::vector<tProvinceIndex> provinceIndices;
	std::vector<Province> provinces;
	std::vector<ShallowTest::Vector2> flow;

	std::atomic<int> drawingFlag;
	std::atomic<int> copyStateFlag;

	enum class Options : int
	{
		None = 0,
		DrawGrid = 0x01,
		DrawProvinceOwnership = 0x02,
		DrawProvinceArmyCount = 0x04,
		DrawVectorField = 0x08,
	};

	Options options = Options::None;
};

void mainDraw(const GameState& gameState, DrawingContext& context);
