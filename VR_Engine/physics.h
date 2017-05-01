#pragma once
#include "Vectors.h"
#include "Mesh.h"
#include <vector>

struct Ball
{
	Vector3 Position;
	Vector3 Velocity;
	Vector3 Acceleration;
};


class PhysicsWorld
{
public:
	PhysicsWorld();
	~PhysicsWorld();


	void Initalize();
	void Update(float dt);
	void Draw();

private:

	Ball m_Ball;
	std::vector<Vector3> m_TowerPositions;
	Mesh m_TowerMesh;


};
