#pragma once
struct vec4
{
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
	float w = 0.0;
};

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

struct vec3_l
{
	long long int x = 0;
	long long int y = 0;
	long long int z = 0;

};
struct vec3
{
	float x = 0.0;
	float y = 0.0;
	float z = 0.0;
};

struct vec2
{
	float x = 0.0;
	float y = 0.0;
};
class Vec3f{
    public:
        float x, y, z;
        Vec3f(){
        x = 0.0;
        y = 0.0;
        z = 0.0;
        }
        Vec3f(float _x, float _y, float _z){
        x = _x;
        y = _y;
        z = _z;
    }
};

//Game addresses
enum CubeOffset
{
	GameController = 0x36B1C8,
	World = 0x2E4,
	EntityMap = 0x4,
	LocalPlayer = 0x8006D0
};
