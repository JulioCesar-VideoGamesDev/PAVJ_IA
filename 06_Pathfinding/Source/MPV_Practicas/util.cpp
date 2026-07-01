#include "util.h"

float convertTo360(float a)
{
	if (a < 0.0f)
	{
		int n = static_cast<int>(fabs(a) / 360);
		a += (n + 1) * 360;
	}
	else if (a > 360)
	{
		int n = static_cast<int>(a / 360);
		a -= n * 360;
	}

	return a;
}

float convertTo180(float a)
{
	a = convertTo360(a);
	if (a > 180)
	{
		a -= 360;
	}
	return a;
}



float sign(float a)
{
	if (a == 0.0f)
		return 0.0f;
	else return (a > 0.0f ? 1.0f : -1.0f);
}