#include "vector2.h"

using namespace ShallowTest;

Vector2 operator*(float val, const Vector2& rhs)
{ 
	return rhs * val; 
}