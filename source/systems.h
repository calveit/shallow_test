#pragma once
#include <algorithm>
#include <assert.h>
#include <execution>
#include <functional>
#include <numeric>

#include "game_state.h"
#include "parallel_for.h"
#include "vector2.h"

#include "../Optick_1.3.1/include/optick.h"


struct VectorSystem
{
	static void RandomUnit(const std::vector<int>& indices, std::vector<ShallowTest::Vector2>& inputoutput)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				inputoutput[i] = ShallowTest::Vector2::RandomUnit();
			});
	}

	static void MulByFloat(const std::vector<int>& indices, const std::vector<float>& floats, std::vector<ShallowTest::Vector2>& inputoutput)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				inputoutput[i] = inputoutput[i] * floats[i];
			});
	}

	static void Add(const std::vector<int>& indices, std::vector<ShallowTest::Vector2>& v1, const std::vector<ShallowTest::Vector2>& v2)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				v1[i] = v1[i] + v2[i];
			});
	}

	static void Normalize(const std::vector<int>& indices, std::vector<ShallowTest::Vector2>& vectors)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				vectors[i].SafeNormalize();
			});
	}


	static void RandomAround(const std::vector<int>& indices, const ShallowTest::Vector2& position, float radius, std::vector<ShallowTest::Vector2>& output)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				output[i] = position + ShallowTest::Vector2::RandomUnit() * ((float)(std::rand() - RAND_MAX / 2) / (float)RAND_MAX) * radius;
			});
	}

	static void DirectionTowards(const std::vector<int>& indices, std::vector<PositionVelocity>& positionVelocity, std::vector<ShallowTest::Vector2>& direction, ShallowTest::Vector2 position)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				direction[i] = position - positionVelocity[i].Position;
				direction[i].SafeNormalize();
			});
	}
};

struct VectorFieldSystem
{
	static void CreateDirections(const std::vector<int>& indices, std::vector<int>& left, std::vector<int>& top, std::vector<int>& right, std::vector<int>& bottom)
	{
		OPTICK_EVENT(__FUNCTION__);
		left.resize(indices.size());
		top.resize(indices.size());
		right.resize(indices.size());
		bottom.resize(indices.size());

		const int cellCount = Constants::gridWidth * Constants::gridHeight;

		parallelFor(indices, [&](int i)
			{
				left[i] = (i % Constants::gridWidth) == 0 ? -1 : i - 1;
				top[i] = (i - Constants::gridWidth) < 0 ? -1 : i - Constants::gridWidth;
				right[i] = ((i + 1) % Constants::gridWidth) == 0 ? -1 : i + 1;
				bottom[i] = (i + Constants::gridWidth) >= cellCount ? -1 : i + Constants::gridHeight;
			});

	}

	static void CreatePressure(const std::vector<int>& indices, const std::vector<Province>& provinces, const ShallowTest::Vector2 position, std::vector<float>& pressure)
	{
		OPTICK_EVENT(__FUNCTION__);

		pressure.resize(indices.size());
		parallelFor(indices, [&](int i)
			{
				float y = (float)(i / Constants::gridWidth);
				float x = (float)(i - y * Constants::gridWidth);
		
				ShallowTest::Vector2 thisPosition{ x * Constants::provinceSize + Constants::provinceSize / 2, y * Constants::provinceSize + Constants::provinceSize / 2 };
				pressure[i] = std::clamp((thisPosition - position).Length(), 0.0f, 200.0f);
			});
	}

	static void CreateFlow(const std::vector<int>& indices, const std::vector<int>& leftIndices, const std::vector<int>& topIndices, const std::vector<int>& rightIndices, const std::vector<int>& bottomIndices,
		const std::vector<float>& pressure, std::vector<ShallowTest::Vector2>& flow, float time)
	{
		OPTICK_EVENT(__FUNCTION__);
		ShallowTest::Vector2 left{ -1, 0 };
		ShallowTest::Vector2 top{ 0, -1 };
		ShallowTest::Vector2 right{ 1, 0 };
		ShallowTest::Vector2 bottom{ 0, 1 };

		flow.resize(indices.size());
		parallelFor(indices, [&](int i)
			{
				float thisPressure = pressure[i];
				int y = i / Constants::gridWidth;
				int x = i - y * Constants::gridWidth;

				float horizontalFactor = (float)x / (float)Constants::gridWidth;
				float verticalFactor = (float)y / (float)Constants::gridHeight;
				float factor1 = std::cos(time / 1000.0f);
				float factor2 = std::sin(time / 1000.0f);
				float verticalMult = std::cos((horizontalFactor) * 3.14f + factor2) + std::cos(verticalFactor * 3.14f);
				float horizontalMult = std::sin((verticalFactor) * 3.14f - 3.14f / 2.0f + factor1) + std::cos(horizontalFactor * 3.14f);

				float leftMult = 0.0f;
				float rightMult = horizontalMult;
				float bottomMult = verticalMult;
				float topMult = 0.0f;
				
				ShallowTest::Vector2 leftFlow = left * (leftIndices[i] == -1 ? 0 : (pressure[leftIndices[i]] - thisPressure));
				ShallowTest::Vector2 topFlow = top * (topIndices[i] == -1 ? 0 : (pressure[topIndices[i]] - thisPressure));
				ShallowTest::Vector2 rightFlow = right * (rightIndices[i] == -1 ? 0 : (pressure[rightIndices[i]] - thisPressure));
				ShallowTest::Vector2 bottomFlow = bottom * (bottomIndices[i] == -1 ? 0 : (pressure[bottomIndices[i]] - thisPressure));
				ShallowTest::Vector2 pressureImpact = (leftFlow + topFlow  + rightFlow * 0.f + bottomFlow * 0.f);
				pressureImpact.SafeNormalize();
				//ShallowTest::Vector2 randomImpact = ShallowTest::Vector2::RandomUnit();
				ShallowTest::Vector2 backgroundImpact = left * leftMult + top * topMult + right * rightMult + bottom * bottomMult;
				backgroundImpact.SafeNormalize();
				flow[i] = backgroundImpact + /*randomImpact + */ pressureImpact * 3.5f;
			});
	}
};


struct ArmyToProvinceAssignmentSystem
{
	static tProvinceIndex GetProvinceIndexForPosition(const ShallowTest::Vector2 position)
	{
		int x = (int)(position.x / Constants::provinceSize);
		int y = (int)(position.y / Constants::provinceSize);

		return y * Constants::gridWidth + x;
	}

	static ShallowTest::Vector2 GetPositionFromProvinceIndex(tProvinceIndex index)
	{
		int y = (index / Constants::gridWidth);
		int x = (index - y * Constants::gridWidth);

		return { (float)x * Constants::provinceSize, (float)y * Constants::provinceSize };
	}

	static void AssignArmies(const std::vector<tProvinceIndex>& provinceIndices, std::vector<Province>& provinces, 
		std::vector<tArmyIndex>& armyIndices, std::vector<Army>& armies, std::vector<tArmyIndex>& armyAssignments, 
		std::vector<std::vector<int>>& armiesPerProvincePerThread, std::vector<int>& armyToCountry)
	{
		OPTICK_EVENT(__FUNCTION__);

		const size_t provinceCount = provinceIndices.size();
		const int batchSize = 65535;

		{
			OPTICK_EVENT("Count");

			splitParallelFor(provinceIndices, 256, [&](auto& range, int batchIndex)
				{
					for (int i : range)
					{
						provinces[i].armyCount._a = 0;
					}
				});

			splitParallelFor(armyIndices, batchSize, [&](auto& range, int batchIndex)
				{
					for (int i : range)
					{
						armies[i].provinceIndex = GetProvinceIndexForPosition(armies[i].position);
						++provinces[armies[i].provinceIndex].armyCount;
					}
				});
		}

		int totalCount = 0;
		{
			OPTICK_EVENT("Total");
			serialFor(provinceIndices, [&](int i)
				{
					provinces[i].armyStartIndex = totalCount;
					totalCount += provinces[i].armyCount._a;
					provinces[i].armyCount._a = 0;
				}
			);
		}

		assert(totalCount == armyIndices.size());
		armyAssignments.resize(totalCount);

#if defined(_DEBUG)
		std::fill(armyAssignments.begin(), armyAssignments.end(), -1);
#endif

		{
			OPTICK_EVENT("Set");

			int batchCount = splitParallelForGetBatchCount(armyIndices, batchSize);
			armiesPerProvincePerThread.resize(provinceIndices.size() * batchCount);

			std::vector<int> batchIndices;
			batchIndices.resize(batchCount);

			{
				OPTICK_EVENT("Armies");
				splitParallelFor(armyIndices, batchSize, [&](auto& range, int batchIndex)
					{
						batchIndices[batchIndex] = batchIndex;
						for (int i : range)
						{
							const int provinceIndex = armies[i].provinceIndex;
							armiesPerProvincePerThread[provinceIndex * batchCount + batchIndex]
								.reserve(std::max(256, (int)armiesPerProvincePerThread[provinceIndex * batchCount + batchIndex].size()));
							armiesPerProvincePerThread[provinceIndex * batchCount + batchIndex].push_back(i);
						}
					});
			}

			{
				{
					OPTICK_EVENT("InitA2C");
					parallelFor(armyIndices, [&](int i)
						{
							armyToCountry[i] = armies[i].getCountryIndex();
						});
				}

				OPTICK_EVENT("Provinces");
				parallelFor(provinceIndices, [&](int i)
					{
						const int armyStartIndex = provinces[i].armyStartIndex;
						std::size_t count = 0;
						{
							OPTICK_EVENT("CountIndices");
							for (int batchIndex = 0; batchIndex < batchCount; ++batchIndex)
							{
								std::vector<int>& provinceArmyIndices = armiesPerProvincePerThread[i * batchCount + batchIndex];
								std::copy(provinceArmyIndices.begin(), provinceArmyIndices.end(), armyAssignments.begin() + armyStartIndex + count);
								count += provinceArmyIndices.size();
							}
						}
						{
							OPTICK_EVENT("AtomicCount");
							provinces[i].armyCount._a = (int)count;
						}

						std::vector<int> countryCounts;
						countryCounts.resize(Constants::maxCountries);

						{
							OPTICK_EVENT("CountArmies");
							for (int armyIndex = 0; armyIndex < count; ++armyIndex)
							{
								++countryCounts[armyToCountry[armyAssignments[armyStartIndex + armyIndex]]];
							}
						}

						int bestCount = 0;
						int bestIndex = -1;
						for (int countryIndex = 0; countryIndex < Constants::maxCountries; ++countryIndex)
						{
							if (countryCounts[countryIndex] > bestCount)
							{
								bestCount = countryCounts[countryIndex];
								bestIndex = countryIndex;
							}
						}

						if (bestIndex >= 0)
						{
							provinces[i].prevCountryIndex = provinces[i].countryIndex;
							provinces[i].countryIndex = bestIndex;
						}
					});
			}
		}
	}
};

struct ProvinceToCountryAssignmentSystem
{
	static void AssignProvinces(const std::vector<tCountryIndex>& countryIndices, std::vector<Country>& countries, const std::vector<tProvinceIndex>& provinceIndices, std::vector<Province>& provinces, std::vector<tProvinceIndex>& provinceAssignments)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(countryIndices, [&](int i)
			{
				countries[i].provinceCount = 0;
				countries[i].provinceStartIndex = 0;
			});

		serialFor(provinceIndices, [&](int i)
			{
				int countryIndex = provinces[i].countryIndex;
				if (countryIndex != -1)
					++countries[countryIndex].provinceCount;
			});

		int totalCount = 0;
		serialFor(countryIndices, [&](int i)
			{
				countries[i].provinceStartIndex = totalCount;
				totalCount += countries[i].provinceCount;
				countries[i].provinceCount = 0;
			});

		provinceAssignments.resize(totalCount);

		serialFor(provinceIndices, [&](int i)
			{
				int provinceCountryIndex = provinces[i].countryIndex;
				if (provinceCountryIndex != -1)
				{
					Country& provinceCountry = countries[provinceCountryIndex];

					provinceAssignments[provinceCountry.provinceStartIndex + provinceCountry.provinceCount] = i;
					++provinceCountry.provinceCount;
				}
			});


		serialFor(countryIndices, [&](int i)
			{
				const int provinceIndex = ArmyToProvinceAssignmentSystem::GetProvinceIndexForPosition(countries[i].position);
				provinces[provinceIndex].countryIndex = i;
			});
	}
};

class ArmyToCountryAssignmentSystem
{
public:
	static void AssignArmies(const std::vector<tCountryIndex>& countryIndices, std::vector<Country>& countries, std::vector<tArmyIndex>& armyIndices, std::vector<Army>& armies)
	{
		OPTICK_EVENT(__FUNCTION__);

		int batchCount = splitParallelForGetBatchCount(armyIndices, 65535);

		std::vector<int> armyCounts;
		armyCounts.resize(batchCount * Constants::maxCountries);
		const int armyCount = (int)armyIndices.size();

		splitParallelFor(armyIndices, 65535, [&](auto& range, int batchIndex)
			{
				for (int i : range)
				{
					++armyCounts[batchIndex * Constants::maxCountries + armies[i].getCountryIndex()];
				}
			});


		for (int i = 0; i < armyCounts.size(); ++i)
		{
			countries[i / batchCount].armyCount._a += armyCounts[i];
		}
	}
};

class CountrySystem
{
public:
	static void CalcPositionFromFlow(const std::vector<tCountryIndex>& indices, std::vector<Country>& countries, const std::vector<ShallowTest::Vector2>& flow, const std::vector<ShallowTest::Vector2>& randomVectors, float delta)
	{
		parallelFor(indices, [&](int i)
			{
				const int provinceIndex = ArmyToProvinceAssignmentSystem::GetProvinceIndexForPosition(countries[i].position);
				const float speed = (float)Constants::countrySpeed * (1.0f - std::clamp(countries[i].provinceCount / 1000.0f, 0.0f, 0.9f));
				countries[i].position = countries[i].position + (flow[provinceIndex] * 0.5f + randomVectors[i] * 0.5f) * delta * (float)speed;

				countries[i].position.x = std::clamp<float>(countries[i].position.x, 0, Constants::screenWidth - 1);
				countries[i].position.y = std::clamp<float>(countries[i].position.y, 0, Constants::screenHeight - 1);
			}
		);
	}
};

class ArmySystem
{
public:
	static void CalcPositionFromFlow(const std::vector<int>& indices, std::vector<Army>& armies, const std::vector<ShallowTest::Vector2>& flow, const std::vector<ShallowTest::Vector2>& randomVectors, float delta)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				ShallowTest::Vector2 velocity = (flow[armies[i].provinceIndex] * 0.5f + randomVectors[i] * 0.5f);
				armies[i].position = armies[i].position + velocity * delta * Constants::armySpeed;
				armies[i].position.x = std::clamp<float>(armies[i].position.x, 0, Constants::screenWidth - 1);
				armies[i].position.y = std::clamp<float>(armies[i].position.y, 0, Constants::screenHeight - 1);

				assert(armies[i].position.x >= 0.0f);
				assert(armies[i].position.y >= 0.0f);
			});
	}


	static void SetValid(const std::vector<int>& indices, bool valid, std::vector<Army>& output)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				output[i].validate();
			});

	}

	static void SetHitPoints(const std::vector<int>& indices, char hitPoints, std::vector<Army>& output)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				output[i].setHitPoints(hitPoints);
			});
	}

	static void SetCountryIndex(const std::vector<int>& indices, int countryIndex, std::vector<Army>& output)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				output[i].setCountryIndex(countryIndex);
			});

	}

	static void GetSpeed(const std::vector<int>& indices, const std::vector<Army>& input, std::vector<float>& output)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				output[i] = Constants::armySpeed * (float)std::rand() / (float)RAND_MAX;
			});
	}


	static void CalculateDesiredPosition(const std::vector<int>& indices, const std::vector<PositionVelocity>& input, float deltaT, std::vector<ShallowTest::Vector2>& output)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				output[i] = input[i].Position + input[i].Velocity * deltaT;
				output[i].x = std::clamp<float>(output[i].x, 0, Constants::screenWidth - 1);
				output[i].y = std::clamp<float>(output[i].y, 0, Constants::screenHeight - 1);
			});
	}

	static void SetPosition(const std::vector<int>& indices, const std::vector<ShallowTest::Vector2>& input, std::vector<Army>& output)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(indices, [&](int i)
			{
				output[i].position = input[i];
			});
	}

	static void MergeKilledAndSpawned(const std::vector<int>& armyIndicesAll, std::vector<int>& armyIndices, std::vector<Army>& armies, const std::vector<Country>& countries, std::vector<int>& killedArmies, std::vector<tCountryIndex>& spawnedArmies)
	{
		OPTICK_EVENT(__FUNCTION__);
		const int spawnCount = (int)spawnedArmies.size();
		const int killedCount = (int)killedArmies.size();

		int killedConsumed = 0;
		int overTheLimitIndex = 0;

		auto reuseArmy = [&](tArmyIndex armyIndex, tCountryIndex countryIndex)
		{
			Army& armyToSpawn = armies[armyIndex];
			armyToSpawn.validate();
			armyToSpawn.setCountryIndex(countryIndex);
			armyToSpawn.setHitPoints(Constants::armyInitialHitPoints);
			armyToSpawn.position = countries[countryIndex].position;
			armyToSpawn.provinceIndex = -1;
		};

		for (int i = 0; i < spawnCount; ++i)
		{
			const tCountryIndex countryIndex = spawnedArmies[i];

			if (killedConsumed < killedCount)
			{
				const tArmyIndex armyIndex = killedArmies[killedConsumed];
				reuseArmy(armyIndex, countryIndex);
				++killedConsumed;
			}
			else
			{
				const tArmyIndex armyIndex = (int)armyIndices.size() + overTheLimitIndex;
				reuseArmy(armyIndex, countryIndex);
				++overTheLimitIndex;
			}
		}

		int shrinkCounter = 0;

		// Remove killed armies
		for (int i = killedConsumed; i < killedCount; ++i)
		{
			const tArmyIndex killedArmyIndex = (int)killedArmies[i];
			const tArmyIndex liveArmyIndex = (int)armyIndices.size() - 1 - (i - killedConsumed);

			if (killedArmyIndex < liveArmyIndex)
			{
				std::swap(armies[killedArmyIndex], armies[liveArmyIndex]);
				++shrinkCounter;
			}
		}

		if (overTheLimitIndex > 0)
		{
			const int oldCount = (int)armyIndices.size();
			const int newArmyCount = oldCount + overTheLimitIndex;
			armyIndices.resize(newArmyCount);
			std::iota(armyIndices.begin() + oldCount, armyIndices.end(), oldCount);
		}
		else if (shrinkCounter > 0)
		{
			armyIndices.resize((int)armyIndices.size() - shrinkCounter);
		}

		killedArmies.clear();
		spawnedArmies.clear();
	}

	static void InitializeIndices(const std::vector<int>& armyIndicesAll, std::vector<int>& armyIndices, std::vector<Army>& armies)
	{
		OPTICK_EVENT(__FUNCTION__);

		std::iota(armyIndices.begin(), armyIndices.end(), 0);
	}
};

class SpawnSystem
{
public:
	static void UpdateFactor(const std::vector<tCountryIndex>& countryIndices, std::vector<Country>& countries, float deltaT)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(countryIndices, [&](int i)
			{
				countries[i].spawnFactor += deltaT * ((float)(std::max((short)1, countries[i].provinceCount)) * (float)Constants::spawnPerProvincePerSecond + (float)Constants::constantSpawnRate);
			});
	}

	static void Spawn(std::vector<tArmyIndex>& armyIndices, std::vector<Army>&armies, const std::vector<tCountryIndex>& countryIndices, std::vector<Country>& countries, float deltaT, std::vector<tCountryIndex>& spawnedArmiesCountByCountry)
	{
		OPTICK_EVENT(__FUNCTION__);
		const int countryCount = (int)countries.size();
		std::atomic<int> totalToSpawn = 0;
		serialFor(countryIndices, [&](int i)
			{
				totalToSpawn += (int)std::round(countries[i].spawnFactor);
			});

		
		int clampedTotalToSpawn = std::min((int)(Constants::maxArmies - armyIndices.size()), totalToSpawn.load());
		float reductionRatio = (float)clampedTotalToSpawn / (float)totalToSpawn.load();

		spawnedArmiesCountByCountry.resize(clampedTotalToSpawn);
		int offset = 0;
		for (int i = 0; i < countryCount; ++i)
		{
			int spawnCount = (int)std::round(countries[i].spawnFactor * reductionRatio);
			if (spawnCount == 0)
				continue;
			spawnCount = std::min(spawnCount, clampedTotalToSpawn - offset);
			std::fill(spawnedArmiesCountByCountry.begin() + offset, spawnedArmiesCountByCountry.begin() + offset + spawnCount, i);
			offset += spawnCount;
			countries[i].spawnFactor = 0.0f;
		}
	}
};

class CombatSystem
{
public:
	static void DamageArmies(const std::vector<tArmyIndex>& armyIndices, std::vector<Army>& armies, const std::vector<Province>& provinces)
	{
		OPTICK_EVENT(__FUNCTION__);
		parallelFor(armyIndices, [&](int i)
			{
				if (provinces[armies[i].provinceIndex].countryIndex >= 0 && armies[i].getCountryIndex() != provinces[armies[i].provinceIndex].countryIndex)
					armies[i].setHitPoints(armies[i].getHitPoints() - 1);

				if (provinces[armies[i].provinceIndex].countryIndex != provinces[armies[i].provinceIndex].prevCountryIndex 
					&& provinces[armies[i].provinceIndex].prevCountryIndex == armies[i].getCountryIndex())
					armies[i].setHitPoints(armies[i].getHitPoints() - 1);

			});
	}

	static void KillArmies(const std::vector<tArmyIndex>& armyIndices, std::vector<Army>& armies, std::vector<int>& indicesToKill)
	{
		OPTICK_EVENT(__FUNCTION__);
		const int armyCount = (int)armyIndices.size();
		for (int i = 0; i < armyCount; ++i)
		{
			if (armies[armyIndices[i]].getHitPoints() == 0)
			{
				armies[armyIndices[i]].invalidate();
				indicesToKill.push_back(armyIndices[i]);
			}
		}
	}
};

class PeriodicTask
{
public:
	PeriodicTask(std::function<void()> fn, float interval)
		: _fn(fn), _interval(interval), _progress(0.0f)
	{}
	void update(float deltaT)
	{
		OPTICK_EVENT(__FUNCTION__);
		_progress += deltaT;
		if (_progress > _interval)
		{
			_progress -= _interval;
			_fn();
		}
	}
private:
	std::function<void()> _fn;
	const float _interval;
	float _progress;
};