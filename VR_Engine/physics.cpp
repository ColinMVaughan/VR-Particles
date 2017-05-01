#include "physics.h"
#include <random>

PhysicsWorld::PhysicsWorld()
{

}

PhysicsWorld::~PhysicsWorld()
{

}

void PhysicsWorld::Initalize()
{
	std::uniform_real_distribution<float> randomFloat(0.0, 1.0);
	std::default_random_engine generator;

	for (int y = 0; y < 100; y++)
	{
		for (int x = 0; x < 100; x++)
		{
			Vector3(x, randomFloat(generator), y);
		}
	}

}
