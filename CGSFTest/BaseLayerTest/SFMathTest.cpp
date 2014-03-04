#include "stdafx.h"
#include "SFMathTest.h"
#include <SFMath.h>

SFMathTest::SFMathTest(void)
{
}


SFMathTest::~SFMathTest(void)
{
}

bool SFMathTest::Run()
{

	SFMath::Vector2D Vec(5.0f, 4.0f);
	SFMath::Vector2D Vec2(5.0f, -4.0f);

	Vec = Vec + Vec2;

	SFMath::Vector2D Vec2d(1.0f, 0.0f);
	SFMath::Vector2D Vec2d2(-3.0f, 4.0f);

	Vec2d2 = Vec2d.Reflection(Vec2d2);

	SFMath::Vector3D Vec3d(5.0f, 4.0f, 7.0f);
	SFMath::Vector3D Vec3d2(5.0f, -4.0f, -4.0f);

	Vec3d = Vec3d + Vec3d2;

	Vec2d += Vec2d;

	Vec3d += Vec3d;

	return true;
}
