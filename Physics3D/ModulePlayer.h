#pragma once
#include "Module.h"
#include "Globals.h"
#include "p2Point.h"

struct PhysVehicle3D;

#define MAX_ACCELERATION 1000.0f
#define TURN_DEGREES 30.0f * DEGTORAD
#define BRAKE_POWER 1000.0f

class ModulePlayer : public Module
{
public:
	ModulePlayer(Application* app, bool start_enabled = true);
	virtual ~ModulePlayer();

	bool Start();
	update_status Update(float dt);
	bool CleanUp();
	void RestartPlayer(int x,int y,int z);
	void OnCollision(PhysBody3D* body1, PhysBody3D* body2);

public:

	PhysVehicle3D* vehicle;
	Cylinder* antena;
	PhysBody3D* antena_body;
	vec3 antena_anchor;
	//btTransform lastCheckpoint;
	btPoint2PointConstraint* frontConstraint;
	btPoint2PointConstraint* rearConstraint;
	float turn;
	float acceleration;
	float brake;
	bool alive;
	vec3 lastCheckpoint;
};