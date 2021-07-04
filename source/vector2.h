#pragma once
#include <math.h>
#include <random>

namespace ShallowTest
{
	class Vector2
	{
	public:
		Vector2() = default;
		Vector2(float x, float y) : x(x), y(y) {};

		Vector2 operator+(const Vector2& rhs) const { return Vector2(x + rhs.x, y + rhs.y); }
		Vector2 operator-(const Vector2& rhs) const { return Vector2(x - rhs.x, y - rhs.y); }
		Vector2 operator*(float rhs) const { return Vector2(x * rhs, y * rhs); }

		float Length() const
		{
			return sqrtf(x * x + y * y);
		}

		void SafeNormalize()
		{
			const float length = Length();
			if (length == 0.0f)
			{
				x = 0.0f;
				y = 0.0f;
				return;
			}

			x /= length;
			y /= length;
		}

		static Vector2 RandomUnit()
		{
			int rndX = (std::rand() - RAND_MAX / 2);
			int rndY = (std::rand() - RAND_MAX / 2);

			Vector2 output((float)rndX, (float)rndY);
			output.SafeNormalize();
			return output;
		}

		float x = 0.0f;
		float y = 0.0f;
	};

	Vector2 operator*(float val, const Vector2& rhs);
}
