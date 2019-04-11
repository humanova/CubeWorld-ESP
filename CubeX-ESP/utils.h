struct Vec2f
{
	float x = 0;
	float y = 0;
};

struct Vec2d
{
	double x = 0;
	double y = 0;
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
};
