#include <chrono>
#include <functional>
#include <string>
#include <thread>
#include <time.h>

#include "drawing.h"
#include "game_state.h"
#include "parallel_for.h"
#include "raylib.h"
#include "raylib_extensions.h"
#include "systems.h"
#include "optick.h"


void main()
{
    std::srand((unsigned int)std::time(nullptr));

    const int screenWidth = Constants::screenWidth;
    const int screenHeight = Constants::screenHeight;

    std::vector<Color> countryColors = generateRandomColors(Constants::maxCountries);


    const float desiredFps = 60.0f;
    const float frameBudgetInMs = 1000.0f / desiredFps;
    const float desiredDeltaTimeInS = 1.0f / (float)desiredFps;
    float actualFrameTimeInMs = 0.0f;

    std::vector<Country> countries;
    std::vector<Army> armies;
    std::vector<Province> provinces;

    countries.resize(Constants::maxCountries);
    armies.resize(Constants::maxArmies);
    const int provinceCount = Constants::screenWidth / Constants::provinceSize * Constants::screenHeight / Constants::provinceSize;
    provinces.resize(provinceCount);

    std::default_random_engine generator;
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    ShallowTest::Vector2 CountryPositions[Constants::maxCountries];
    for (int i = 0; i < Constants::maxCountries; i++)
    {
        CountryPositions[i] = { screenWidth / 2, screenHeight / 2 };
        ShallowTest::Vector2 offset;
        offset.x = distribution(generator) * (screenWidth / 2 - 150.0f);
        offset.y = distribution(generator) * (screenHeight / 2 - 150.0f);
        CountryPositions[i] = CountryPositions[i] + offset;

        countries[i].position = CountryPositions[i];
        provinces[ArmyToProvinceAssignmentSystem::GetProvinceIndexForPosition(CountryPositions[i])].countryIndex = i;
    }

    std::vector<ShallowTest::Vector2> armyVectors1; armyVectors1.resize(Constants::maxArmies);
    std::vector<ShallowTest::Vector2> armyVectors2; armyVectors2.resize(Constants::maxArmies);
    std::vector<float> armyFloats; armyFloats.resize(Constants::maxArmies);
    std::vector<int> armyInts; armyInts.resize(Constants::maxArmies);

    std::vector<tArmyIndex> armyToProvinceAssignments;
    std::vector<tArmyIndex> armyToCountryAssignments;
    std::vector<tProvinceIndex> provinceToCountryAssignments;

    std::vector<tArmyIndex> validArmyIndices; validArmyIndices.resize(Constants::maxArmies);
    std::iota(validArmyIndices.begin(), validArmyIndices.end(), 0);
    const std::vector<tArmyIndex> armyIndicesAll = validArmyIndices;
    validArmyIndices.resize(Constants::maxCountries * Constants::initialArmiesPerCountry);

    std::vector<tProvinceIndex> provinceIndices; provinceIndices.resize(provinces.size());
    std::iota(provinceIndices.begin(), provinceIndices.end(), 0);

    std::vector<tCountryIndex> countryIndices; countryIndices.resize(Constants::maxCountries);
    std::iota(countryIndices.begin(), countryIndices.end(), 0);

    std::vector<tArmyIndex> killedArmiesIndices;
    std::vector<tArmyIndex> spawnedArmiesCountByCountry;

    std::vector<int> left, top, right, bottom;
    VectorFieldSystem::CreateDirections(provinceIndices, left, top, right, bottom);

    std::vector<float> pressure;
    pressure.resize(provinceIndices.size());

    std::vector<ShallowTest::Vector2> flow;
    flow.resize(provinceIndices.size());

    int armiesPerCountry = Constants::initialArmiesPerCountry;
    std::for_each(std::execution::par_unseq, countryIndices.begin(), countryIndices.end(), [&](int i)
        {
            const auto indices_slice = slice(validArmyIndices, i * armiesPerCountry, i * armiesPerCountry + armiesPerCountry - 1);
            ArmySystem::SetCountryIndex(indices_slice, i, armies);
            ArmySystem::SetValid(indices_slice, true, armies);
            ArmySystem::SetHitPoints(indices_slice, Constants::armyInitialHitPoints, armies);
            VectorSystem::RandomAround(indices_slice, CountryPositions[i], 150, armyVectors1);
            ArmySystem::SetPosition(indices_slice, armyVectors1, armies);
        });

    std::vector<int> armyIndiciesCopy;
    std::vector<Army> armiesCopy;
    armiesCopy.resize(Constants::maxArmies);
    std::vector<Province> provincesCopy;
    provincesCopy.resize(provinces.size());

    const char randomSetCount = 5;
    std::vector<ShallowTest::Vector2> randomVectors[randomSetCount];

    for (int i = 0; i < randomSetCount; ++i)
    {
        randomVectors[i].resize(armyIndicesAll.size());
        VectorSystem::RandomUnit(armyIndicesAll, randomVectors[i]);
    }

    char currentRandomSet = 0;

    GameState drawingStateCopy{ 
        std::cref(validArmyIndices), 
        std::cref(armies), 
        std::cref(countryIndices), 
        std::cref(countries), 
        std::cref(countryColors),
        std::cref(provinceIndices), 
        std::cref(provinces), 
        std::cref(flow) 
    };

    DrawingContext drawingContext;
    drawingContext.options = DrawingContext::Options::DrawVectorField;

    std::thread drawingThread([&]()
        {
            mainDraw(drawingStateCopy, drawingContext);
        });

    PeriodicTask spawnTask([&]{ SpawnSystem::Spawn(validArmyIndices, armies, countryIndices, countries, desiredDeltaTimeInS, spawnedArmiesCountByCountry); }, 0.01f);
    float timePassed = 0.0f;
    float mouseInteractionRadius = (float)Constants::interactionRadius;

    while (true)
    {
        OPTICK_FRAME("Main Thread");
        drawingContext.drawingFlag = 1;

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

        const bool spaceDown = IsKeyDown(KEY_SPACE);

        if (!spaceDown)
        {
            ArmySystem::MergeKilledAndSpawned(armyIndicesAll, validArmyIndices, armies, countries, killedArmiesIndices, spawnedArmiesCountByCountry);

            ArmySystem::InitializeIndices(armyIndicesAll, validArmyIndices, armies);

            SpawnSystem::UpdateFactor(countryIndices, countries, desiredDeltaTimeInS);

            std::vector<std::vector<int>> armiesPerProvincePerThread;
            ArmyToProvinceAssignmentSystem::AssignArmies(provinceIndices, provinces, validArmyIndices, armies, armyToProvinceAssignments, armiesPerProvincePerThread, armyInts);

            std::thread cleanup([&]()
                {
                    auto releasee = std::move(armiesPerProvincePerThread);
                    releasee.clear();
                });

            ProvinceToCountryAssignmentSystem::AssignProvinces(countryIndices, countries, provinceIndices, provinces, provinceToCountryAssignments);
            ArmyToCountryAssignmentSystem::AssignArmies(countryIndices, countries, validArmyIndices, armies);

            ArmySystem::CalcPositionFromFlow(validArmyIndices, armies, flow, randomVectors[currentRandomSet], desiredDeltaTimeInS);
            CountrySystem::CalcPositionFromFlow(countryIndices, countries, flow, randomVectors[currentRandomSet], desiredDeltaTimeInS);

            Vector2 mousePos = GetMousePosition();
            mouseInteractionRadius = std::clamp(mouseInteractionRadius + GetMouseWheelMove() * 5.0f, (float)Constants::minInteractionRadius, (float)Constants::maxInteractionRadius);

            {
                OPTICK_EVENT("Wait for drawing");
                // Wait for drawing thread
                while (drawingContext.drawingFlag)
                    std::this_thread::yield();

                drawingContext.interactionRadius = mouseInteractionRadius;
                drawingContext.copyStateFlag = 1;
            }           

            CombatSystem::DamageArmies(validArmyIndices, armies, provinces);
            if (IsMouseButtonDown(1))
                CombatSystem::DamageArmiesWithinRadius(validArmyIndices, armies, { mousePos.x, mousePos.y }, mouseInteractionRadius);
    
            CombatSystem::KillArmies(validArmyIndices, armies, killedArmiesIndices);
            spawnTask.update(desiredDeltaTimeInS);
            
            if (IsMouseButtonDown(0))
                VectorFieldSystem::CreatePressure(provinceIndices, provinces, { mousePos.x, mousePos.y }, mouseInteractionRadius, pressure);
            else
                VectorFieldSystem::ClearPressure(provinceIndices, provinces, pressure);

            VectorFieldSystem::CreateFlow(provinceIndices, left, top, right, bottom, pressure, flow, timePassed);

            cleanup.join();
        }
        else
        {
            OPTICK_EVENT("Wait for drawing");
            // Wait for drawing thread
            while (drawingContext.drawingFlag)
                std::this_thread::yield();

            drawingContext.copyStateFlag = 1;
        }


        {
            OPTICK_EVENT("Wait for copy");
            while (drawingContext.copyStateFlag)
                std::this_thread::yield();
        }

        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        actualFrameTimeInMs = (float)std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count();
        timePassed += actualFrameTimeInMs;

        currentRandomSet = (std::rand() % randomSetCount);
    }
}