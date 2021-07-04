#pragma once


struct Constants
{
	static const int screenWidth = 1920;
	static const int screenHeight = 1080;
	static const int provinceSize = 30;

	static const int gridWidth = screenWidth / provinceSize;
	static const int gridHeight = screenHeight / provinceSize;

	static const int initialArmiesPerCountry = 66666;
	static const unsigned char armyInitialHitPoints = 50;
	static const int armySpeed = 270;
	static const int countrySpeed = 1570;

	static const int maxCountries = 15;
	static const int maxArmies = 1000000;

	static const int spawnPerProvincePerSecond = 50;
	static const int constantSpawnRate = 1000;
};

static_assert(Constants::screenWidth% Constants::provinceSize == 0);
static_assert(Constants::screenHeight% Constants::provinceSize == 0);
static_assert(Constants::initialArmiesPerCountry <= Constants::maxArmies / Constants::maxCountries);