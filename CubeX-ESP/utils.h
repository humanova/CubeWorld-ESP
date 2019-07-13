#pragma once

template <typename T>
class Vector3
{
public:
	T X = 0;
	T Y = 0;
	T Z = 0;

	Vector3() {};
	Vector3(T x, T y, T z) : X(x), Y(y), Z(z) {};
	~Vector3() {};
};

template <typename T>
class Vector2
{
public:
	T X = 0;
	T Y = 0;

	Vector2() {};
	Vector2(T x, T y) : X(x), Y(y){};
	~Vector2() {};
};

struct vec3
{
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
};

struct vec4
{
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
	float w = 0.0;
};

struct vec2
{
	float x = 0.0;
	float y = 0.0;
};

//Game addresses
enum CubeOffset
{
	GameController = 0x36B1C8,
	CameraDistance = 0x1C0,
	CameraPosition = 0x140,
	CameraFocusPosition = 0x14C,
	CameraAngle = 0x1B0,
	CameraAngleCurr = 0x1A4,
	CursorEntityGUID = 0x800A70,
	World = 0x2E4,
	WorldTime = 0x80015C,
	EntityMap = 0x4,
	LocalPlayer = 0x8006D0
};
