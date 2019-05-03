#include "Creature.h"
#include <math.h>

double Creature::DistanceFrom(Vector3<int64_t> point) {
	auto p1 = this->entity_data.position;
	auto p2 = point;

	auto x = p1.X - p2.X;
	auto y = p1.Y - p2.Y;
	auto z = p1.Z - p2.Z;

	return sqrt((x*x) + (y*y) + (z*z));
}
double Creature::DistanceFrom(Creature* target) {
	return DistanceFrom(target->entity_data.position);
}