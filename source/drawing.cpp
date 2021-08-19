#include "drawing.h"

#include <algorithm>
#include <atomic>
#include <bitset>
#include <numeric>
#include <string>
#include <vector>
#include <thread>

#include "constants.h"
#include "optick.h"
#include "parallel_for.h"
#include "raylib.h"
#include "raylib_extensions.h"


using Opt = DrawingContext::Options;

template<typename E>
struct enable_bitmask_operators {
    static constexpr bool enable = false;
};

template<>
struct enable_bitmask_operators<DrawingContext::Options> {
    static constexpr bool enable = true;
};

template<typename E>
typename std::enable_if<enable_bitmask_operators<E>::enable, E>::type
operator&(E lhs, E rhs) {
    typedef typename std::underlying_type<E>::type underlying;
    return static_cast<E>(
        static_cast<underlying>(lhs) & static_cast<underlying>(rhs));
}

void mainDraw(const GameState& gameState, DrawingContext& context)
{
    InitWindow(Constants::screenWidth, Constants::screenHeight, "Shallow Update Test");
    SetTargetFPS(60);

    // Load star texture
    Texture2D starTexture = LoadTexture("star.png");
    Image starImage = GetTextureData(starTexture);
    Color* starRaw = (Color*)starImage.data;

    std::vector<Color*> countryRaw;
    countryRaw.resize(Constants::maxCountries);

    for (int i = 0; i < Constants::maxCountries; ++i)
    {
        countryRaw[i] = new Color[starImage.width * starImage.height];
        for (int pixel = 0; pixel < starImage.width * starImage.height; ++pixel)
        {
            countryRaw[i][pixel] = starRaw[pixel] * gameState.countryColors.get()[i];
        }
    }

    while (!WindowShouldClose())
    {
        OPTICK_THREAD("DrawingThread");

        {
            OPTICK_EVENT("Wait");
            while (!context.drawingFlag)
                std::this_thread::yield();
        }

        Texture armiesTex;

        static std::vector<char> deadArmies;
        static std::vector<int> deadArmiesIndices;
        if (deadArmies.size() == 0)
        {
            deadArmies.resize(Constants::screenWidth * Constants::screenHeight);
            deadArmiesIndices.resize(Constants::screenWidth * Constants::screenHeight);
            std::fill(deadArmies.begin(), deadArmies.end(), 0);
            std::iota(deadArmiesIndices.begin(), deadArmiesIndices.end(), 0);
        }


        {
            OPTICK_EVENT("Generate Texture");

            Color* pixels = new Color[Constants::screenWidth * Constants::screenHeight];
            std::fill(pixels, pixels + Constants::screenWidth * Constants::screenHeight, BLACK);
            std::bitset<Constants::maxArmies> bitset;

            {
                OPTICK_EVENT("Armies");
                parallelFor(context.armyIndices, [&](int index)
                    {
                        int i = context.armyIndices[index];
                        int screenIndex = Constants::screenWidth * (int)context.armies[i].position.y + (int)context.armies[i].position.x;

                        char hitPointFactor = Constants::armyInitialHitPoints - context.armies[i].getHitPoints() + 1;
                        const int countryIndex = context.armies[i].getCountryIndex();
                        Color c = gameState.countryColors.get()[countryIndex];
                        c.a = 150;

                        int x = (int)context.armies[i].position.x;
                        int y = (int)context.armies[i].position.y;
                        int pixelIndex = y * Constants::screenWidth + x;

                        if (context.armies[i].getHitPoints() == 0)
                        {
                            deadArmies[pixelIndex] = 15;
                        }

                        
                        pixels[pixelIndex] += c;

                        const bool right = pixelIndex + 1 < Constants::screenWidth * Constants::screenHeight;
                        const bool left = pixelIndex > 0;
                        const bool bottom = pixelIndex + Constants::screenWidth < Constants::screenWidth* Constants::screenHeight;
                        const bool top = pixelIndex - Constants::screenWidth >= 0;

                        if (right)
                            pixels[pixelIndex + 1] += c;
                        if (left)
                            pixels[pixelIndex - 1] += c;
                        if (bottom)
                            pixels[pixelIndex + Constants::screenWidth] += c;
                        if (top)
                            pixels[pixelIndex - Constants::screenWidth] += c;

                        int screenHBegin = std::max(0, x - starImage.width / 2); //std::max(0, starImage.width - x);
                        int screenVBegin = std::max(0, y - starImage.height / 2); //std::max(0, starImage.height - y);
                        int screenHEnd = std::min(Constants::screenWidth, x - starImage.width / 2 + starImage.width);
                        int screenVEnd = std::min(Constants::screenHeight, y - starImage.height / 2+ starImage.height);

                        /*
                        for (int line = screenVBegin; line < screenVEnd; ++line)
                        {
                            int index = line * Constants::screenWidth + screenHBegin;
                            int end = index + (screenHEnd - screenHBegin);
                            std::copy(countryRaw[countryIndex] + (line - screenVBegin) * starImage.width, countryRaw[countryIndex] + (line + 1 - screenVBegin) * starImage.width, pixels + index);
                        }
                        */
                        bitset.set(index);
                    });

                /*
                parallelFor(deadArmiesIndices, [&](int pixelIndex)
                    {
                        if (deadArmies[pixelIndex] > 0)
                        {
                            Color c = BLACK;
                            c.r = (char)(255.0f * (float)deadArmies[pixelIndex] / 15);

                            pixels[pixelIndex] += c;

                            const bool right = pixelIndex + 1 < Constants::screenWidth * Constants::screenHeight;
                            const bool left = pixelIndex > 0;
                            const bool bottom = pixelIndex + Constants::screenWidth < Constants::screenWidth* Constants::screenHeight;
                            const bool top = pixelIndex - Constants::screenWidth >= 0;

                            if (right)
                                pixels[pixelIndex + 1] += c;
                            if (left)
                                pixels[pixelIndex - 1] += c;
                            if (bottom)
                                pixels[pixelIndex + Constants::screenWidth] += c;
                            if (top)
                                pixels[pixelIndex - Constants::screenWidth] += c;

                            --deadArmies[pixelIndex];
                        }
                    });
                    */

            }

            for (int index = 0; index < context.countryIndices.size(); ++index)
            {
                int i = context.countryIndices[index];
                int pixelIndex = Constants::screenWidth * (int)context.countries[i].position.y + (int)context.countries[i].position.x;
                Color c = gameState.countryColors.get()[i];

                pixels[pixelIndex] = c;

                bool right = pixelIndex + 1 < Constants::screenWidth * Constants::screenHeight;
                bool left = pixelIndex > 0;
                bool bottom = pixelIndex + Constants::screenWidth < Constants::screenWidth* Constants::screenHeight;
                bool top = pixelIndex - Constants::screenWidth >= 0;
                bool topLeft = pixelIndex - Constants::screenWidth - 1 > 0;
                bool topRight = pixelIndex - Constants::screenWidth + 1 > 0;
                bool bottomLeft = pixelIndex + Constants::screenWidth - 1 < Constants::screenHeight * Constants::screenWidth;
                bool bottomRight = pixelIndex + Constants::screenWidth + 1 < Constants::screenHeight * Constants::screenWidth;

                if (right)
                    pixels[pixelIndex + 1] = c;
                if (left)
                    pixels[pixelIndex - 1] = c;
                if (bottom)
                    pixels[pixelIndex + Constants::screenWidth] = c;
                if (top)
                    pixels[pixelIndex - Constants::screenWidth] = c;
                if (topLeft)
                    pixels[pixelIndex - Constants::screenWidth - 1] = c;
                if (topRight)
                    pixels[pixelIndex - Constants::screenWidth + 1] = c;
                if (bottomLeft)
                    pixels[pixelIndex + Constants::screenWidth - 1] = c;
                if (bottomRight)
                    pixels[pixelIndex + Constants::screenWidth + 1] = c;
            }

            Image armiesImg
            {
                pixels,
                Constants::screenWidth,
                Constants::screenHeight,
                1,
                PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
            };

            armiesTex = LoadTextureFromImage(armiesImg);
            UnloadImage(armiesImg);
        }

        {
            OPTICK_EVENT("Drawing");
            // Draw
            BeginDrawing();
            ClearBackground(BLACK);

            const int provinceCount = (int)context.provinces.size();
            const int provincesInRow = (Constants::screenWidth / Constants::provinceSize);

            DrawTexture(armiesTex, 0, 0, WHITE);

            auto drawArrow = [&](ShallowTest::Vector2 from, ShallowTest::Vector2 to)
            {
                DrawLine((int)from.x, (int)from.y, (int)to.x, (int)to.y, WHITE);
                DrawRectangle((int)to.x - 1, (int)to.y - 1, 3, 3, WHITE);

            };

            for (int i = 0; i < provinceCount && (context.options != Opt::None); i++)
            {
                int y = i / provincesInRow;
                int x = i - y * provincesInRow;

                Color c = context.provinces[i].countryIndex == -1 ? BLACK : gameState.countryColors.get()[context.provinces[i].countryIndex];

                if ((context.options & Opt::DrawProvinceOwnership) == Opt::DrawProvinceOwnership)
                {
                    c.a = 200;
                    if (context.provinces[i].countryIndex != -1)
                        DrawRectangle(x * Constants::provinceSize, y * Constants::provinceSize, Constants::provinceSize, Constants::provinceSize, c);
                }

                if ((context.options & Opt::DrawProvinceArmyCount) == Opt::DrawProvinceArmyCount)
                {
                    c = WHITE;
                    c.a = std::clamp(context.provinces[i].armyCount._a.load(), 0, 255);
                    DrawRectangle(x * Constants::provinceSize, y * Constants::provinceSize, Constants::provinceSize, Constants::provinceSize, c);
                }

                if ((context.options & Opt::DrawVectorField) == Opt::DrawVectorField)
                {
                    drawArrow(
                        { (float)x * Constants::provinceSize + Constants::provinceSize / 2, (float)y * Constants::provinceSize + Constants::provinceSize / 2 },
                        { x * Constants::provinceSize + Constants::provinceSize / 2 + context.flow[i].x * Constants::provinceSize / 2,
                        y * Constants::provinceSize + Constants::provinceSize / 2 + context.flow[i].y * Constants::provinceSize / 2 });
                }
            }

            if ((context.options & Opt::DrawGrid) == Opt::DrawGrid)
            {
                for (int i = 0; i < Constants::screenWidth / Constants::provinceSize; i++)
                    DrawLine(i * Constants::provinceSize, 0, i * Constants::provinceSize, Constants::screenHeight, DARKGRAY);
                for (int i = 0; i < Constants::screenHeight / Constants::provinceSize; i++)
                    DrawLine(0, i * Constants::provinceSize, Constants::screenWidth, i * Constants::provinceSize, DARKGRAY);
            }

            DrawFPS(10, 10);
            DrawText((std::to_string(context.armyIndices.size() / 1000) + "k dots").c_str(), 120, 10, 20, YELLOW);

            for (int i = 0; i < context.countryIndices.size(); ++i)
            {
                DrawText((std::to_string(context.countries[i].armyCount._a) + " dots").c_str(), 10, 35 + i * 20, 15, gameState.countryColors.get()[i]);
            }
            
            DrawCircleLines(GetMouseX(), GetMouseY(), context.interactionRadius, GRAY);

            EndDrawing();
        }

        UnloadTexture(armiesTex);

        context.drawingFlag = 0;

        {
            OPTICK_EVENT("Wait");
            while (!context.copyStateFlag)
                std::this_thread::yield();
        }

        {
            OPTICK_EVENT("CopyState");

            context.armyIndices = gameState.armyIndices;
            context.countryIndices = gameState.countryIndices;
            context.provinceIndices = gameState.provinceIndices;
            context.armies.resize(context.armyIndices.size());
            context.provinces.resize(context.provinceIndices.size());
            context.flow.resize(context.provinceIndices.size());
            context.countries.resize(context.countryIndices.size());
            slice(gameState.armies.get(), 0, (int)gameState.armyIndices.get().size() - 1, context.armies);
            slice(gameState.provinces.get(), 0, (int)gameState.provinceIndices.get().size() - 1, context.provinces);
            slice(gameState.countries.get(), 0, (int)gameState.countryIndices.get().size() - 1, context.countries);
            slice(gameState.flow.get(), 0, (int)gameState.provinceIndices.get().size() - 1, context.flow);
        }

        context.copyStateFlag = 0;
    }
}