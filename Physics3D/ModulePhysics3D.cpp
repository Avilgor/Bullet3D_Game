#include "Globals.h"
#include "Application.h"
#include "ModulePhysics3D.h"
#include "PhysBody3D.h"
#include "PhysVehicle3D.h"
#include "Primitive.h"

#ifdef _DEBUG
	#pragma comment (lib, "Bullet/libx86/BulletDynamics_debug.lib")
	#pragma comment (lib, "Bullet/libx86/BulletCollision_debug.lib")
	#pragma comment (lib, "Bullet/libx86/LinearMath_debug.lib")
#else
	#pragma comment (lib, "Bullet/libx86/BulletDynamics.lib")
	#pragma comment (lib, "Bullet/libx86/BulletCollision.lib")
	#pragma comment (lib, "Bullet/libx86/LinearMath.lib")
#endif

ModulePhysics3D::ModulePhysics3D(Application* app, bool start_enabled) : Module(app, start_enabled)
{
	debug = true;

	collision_conf = new btDefaultCollisionConfiguration();
	dispatcher = new btCollisionDispatcher(collision_conf);
	broad_phase = new btDbvtBroadphase();
	solver = new btSequentialImpulseConstraintSolver();
	debug_draw = new DebugDrawer();
}

// Destructor
ModulePhysics3D::~ModulePhysics3D()
{
	delete debug_draw;
	delete solver;
	delete broad_phase;
	delete dispatcher;
	delete collision_conf;
}

// Render not available yet----------------------------------
bool ModulePhysics3D::Init()
{
	LOG("Creating 3D Physics simulation");
	bool ret = true;

	return ret;
}

// ---------------------------------------------------------
bool ModulePhysics3D::Start()
{
	LOG("Creating Physics environment");

	world = new btDiscreteDynamicsWorld(dispatcher, broad_phase, solver, collision_conf);
	world->setDebugDrawer(debug_draw);
	world->setGravity(GRAVITY);
	vehicle_raycaster = new btDefaultVehicleRaycaster(world);

	// Big plane as ground
	{
		btCollisionShape* colShape = new btStaticPlaneShape(btVector3(0, 1, 0), 0);

		btDefaultMotionState* myMotionState = new btDefaultMotionState();
		btRigidBody::btRigidBodyConstructionInfo rbInfo(0.0f, myMotionState, colShape);

		btRigidBody* body = new btRigidBody(rbInfo);
		world->addRigidBody(body);
	}

	return true;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::PreUpdate(float dt)
{
	world->stepSimulation(dt, 15);

	int numManifolds = world->getDispatcher()->getNumManifolds();
	for(int i = 0; i<numManifolds; i++)
	{
		btPersistentManifold* contactManifold = world->getDispatcher()->getManifoldByIndexInternal(i);
		btCollisionObject* obA = (btCollisionObject*)(contactManifold->getBody0());
		btCollisionObject* obB = (btCollisionObject*)(contactManifold->getBody1());

		int numContacts = contactManifold->getNumContacts();
		if(numContacts > 0)
		{
			PhysBody3D* pbodyA = (PhysBody3D*)obA->getUserPointer();
			PhysBody3D* pbodyB = (PhysBody3D*)obB->getUserPointer();

			if(pbodyA && pbodyB)
			{
				p2List_item<Module*>* item = pbodyA->collision_listeners.getFirst();
				while(item)
				{
					item->data->OnCollision(pbodyA, pbodyB);
					item = item->next;
				}

				item = pbodyB->collision_listeners.getFirst();
				while(item)
				{
					item->data->OnCollision(pbodyB, pbodyA);
					item = item->next;
				}
			}
		}
	}

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::Update(float dt)
{
	if(App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		debug = !debug;

	if(debug == true)
	{
		world->debugDrawWorld();

		// Render vehicles
		p2List_item<PhysVehicle3D*>* item = vehicles.getFirst();
		while(item)
		{
			item->data->Render();
			item = item->next;
		}

		if(App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN)
		{
			Sphere s(1);
			s.SetPos(App->camera->Position.x, App->camera->Position.y, App->camera->Position.z);
			float force = 30.0f;
			AddBody(s)->Push(-(App->camera->Z.x * force), -(App->camera->Z.y * force), -(App->camera->Z.z * force));
		}
	}

	return UPDATE_CONTINUE;
}

// ---------------------------------------------------------
update_status ModulePhysics3D::PostUpdate(float dt)
{
	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModulePhysics3D::CleanUp()
{
	LOG("Destroying 3D Physics simulation");

	// Remove from the world all collision bodies
	for(int i = world->getNumCollisionObjects() - 1; i >= 0; i--)
	{
		btCollisionObject* obj = world->getCollisionObjectArray()[i];
		world->removeCollisionObject(obj);
	}

	for(p2List_item<btTypedConstraint*>* item = constraints.getFirst(); item; item = item->next)
	{
		world->removeConstraint(item->data);
		delete item->data;
	}
	
	constraints.clear();

	for(p2List_item<btDefaultMotionState*>* item = motions.getFirst(); item; item = item->next)
		delete item->data;

	motions.clear();

	for(p2List_item<btCollisionShape*>* item = shapes.getFirst(); item; item = item->next)
		delete item->data;

	shapes.clear();

	for(p2List_item<PhysBody3D*>* item = bodies.getFirst(); item; item = item->next)
		delete item->data;

	bodies.clear();

	for(p2List_item<PhysVehicle3D*>* item = vehicles.getFirst(); item; item = item->next)
		delete item->data;

	vehicles.clear();

	delete vehicle_raycaster;
	delete world;

	return true;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody( Sphere& sphere, float mass, CollisionObject coll, float x, float y, float z)
{
	btCollisionShape* colShape = new btSphereShape(sphere.radius);
	shapes.add(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&sphere.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.add(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	
	pbody->collType = coll;
	pbody->checkpointX = x;
	pbody->checkpointY = y;
	pbody->checkpointZ = z;
	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.add(pbody);
	//sphere.body = pbody;
	mat4x4 matrix;
	pbody->GetTransform(&matrix);
	sphere.transform = matrix;
	return pbody;
}


// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody( Cube& cube, float mass, CollisionObject coll,float x,float y,float z)
{
	btCollisionShape* colShape = new btBoxShape(btVector3(cube.size.x*0.5f, cube.size.y*0.5f, cube.size.z*0.5f));
	shapes.add(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&cube.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.add(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);

	
	pbody->collType = coll;
	pbody->checkpointX = x;
	pbody->checkpointY = y;
	pbody->checkpointZ = z;
	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.add(pbody);
	//cube.body = pbody;
	mat4x4 matrix;
	pbody->GetTransform(&matrix);
	cube.transform = matrix;

	return pbody;
}

// ---------------------------------------------------------
PhysBody3D* ModulePhysics3D::AddBody( Cylinder& cylinder, float mass, CollisionObject coll, float x, float y, float z)
{
	btCollisionShape* colShape = new btCylinderShapeX(btVector3(cylinder.height*0.5f, cylinder.radius, 0.0f));
	shapes.add(colShape);

	btTransform startTransform;
	startTransform.setFromOpenGLMatrix(&cylinder.transform);

	btVector3 localInertia(0, 0, 0);
	if(mass != 0.f)
		colShape->calculateLocalInertia(mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	motions.add(myMotionState);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(mass, myMotionState, colShape, localInertia);

	btRigidBody* body = new btRigidBody(rbInfo);
	PhysBody3D* pbody = new PhysBody3D(body);
	
	pbody->collType = coll;
	pbody->checkpointX = x;
	pbody->checkpointY = y;
	pbody->checkpointZ = z;
	body->setUserPointer(pbody);
	world->addRigidBody(body);
	bodies.add(pbody);
	//cylinder.body = pbody;
	mat4x4 matrix;
	pbody->GetTransform(&matrix);
	cylinder.transform = matrix;

	return pbody;
}

// ---------------------------------------------------------
PhysVehicle3D* ModulePhysics3D::AddVehicle(const VehicleInfo& info)
{
	btCompoundShape* comShape = new btCompoundShape();
	shapes.add(comShape);

	btCollisionShape* colShape = new btBoxShape(btVector3(info.chassis_size.x*0.5f, info.chassis_size.y*0.5f, info.chassis_size.z*0.5f));
	shapes.add(colShape);

	btTransform trans;
	trans.setIdentity();
	trans.setOrigin(btVector3(info.chassis_offset.x, info.chassis_offset.y, info.chassis_offset.z));

	comShape->addChildShape(trans, colShape);

	btTransform startTransform;
	startTransform.setIdentity();

	btVector3 localInertia(0, 0, 0);
	comShape->calculateLocalInertia(info.mass, localInertia);

	btDefaultMotionState* myMotionState = new btDefaultMotionState(startTransform);
	btRigidBody::btRigidBodyConstructionInfo rbInfo(info.mass, myMotionState, comShape, localInertia);

	btRigidBody* _body = new btRigidBody(rbInfo);
	_body->setContactProcessingThreshold(BT_LARGE_FLOAT);
	_body->setActivationState(DISABLE_DEACTIVATION);

	world->addRigidBody(_body);
	_body->collType = CAR;

	btRaycastVehicle::btVehicleTuning tuning;
	tuning.m_frictionSlip = info.frictionSlip;
	tuning.m_maxSuspensionForce = info.maxSuspensionForce;
	tuning.m_maxSuspensionTravelCm = info.maxSuspensionTravelCm;
	tuning.m_suspensionCompression = info.suspensionCompression;
	tuning.m_suspensionDamping = info.suspensionDamping;
	tuning.m_suspensionStiffness = info.suspensionStiffness;

	btRaycastVehicle* vehicle = new btRaycastVehicle(tuning, _body, vehicle_raycaster);

	vehicle->setCoordinateSystem(0, 1, 2);

	for(int i = 0; i < info.num_wheels; ++i)
	{
		btVector3 conn(info.wheels[i].connection.x, info.wheels[i].connection.y, info.wheels[i].connection.z);
		btVector3 dir(info.wheels[i].direction.x, info.wheels[i].direction.y, info.wheels[i].direction.z);
		btVector3 axis(info.wheels[i].axis.x, info.wheels[i].axis.y, info.wheels[i].axis.z);

		vehicle->addWheel(conn, dir, axis, info.wheels[i].suspensionRestLength, info.wheels[i].radius, tuning, info.wheels[i].front);
	}
	// ---------------------

	PhysVehicle3D* pvehicle = new PhysVehicle3D(_body, vehicle, info);
	_body->setUserPointer(pvehicle);
	//pvehicle->body = _body;
	world->addVehicle(vehicle);
	vehicles.add(pvehicle);

	return pvehicle;
}

// ---------------------------------------------------------
void ModulePhysics3D::AddConstraintP2P(btRigidBody& bodyA, btRigidBody& bodyB, const vec3& anchorA, const vec3& anchorB)
{
	btTypedConstraint* p2p = new btPoint2PointConstraint(
		bodyA, 
		bodyB, 
		btVector3(anchorA.x, anchorA.y, anchorA.z), 
		btVector3(anchorB.x, anchorB.y, anchorB.z));
	world->addConstraint(p2p);
	constraints.add(p2p);
	p2p->setDbgDrawSize(2.0f);
}


void ModulePhysics3D::AddConstraintHinge(PhysBody3D& bodyA, PhysBody3D& bodyB, const vec3& anchorA, const vec3& anchorB, const vec3& axisA, const vec3& axisB, bool disable_collision)
{
	btHingeConstraint* hinge = new btHingeConstraint(
		*(bodyA.body), 
		*(bodyB.body), 
		btVector3(anchorA.x, anchorA.y, anchorA.z),
		btVector3(anchorB.x, anchorB.y, anchorB.z),
		btVector3(axisA.x, axisA.y, axisA.z), 
		btVector3(axisB.x, axisB.y, axisB.z));

	world->addConstraint(hinge, disable_collision);
	constraints.add(hinge);
	hinge->setDbgDrawSize(2.0f);
}

void ModulePhysics3D::RectRoad(int length,int width, int x,int y,int z, RoadTypes direction)
{
	bool colorSwitch = false;
	for (int n = 0; n < length*2; n++)
	{
		Cube* left = new Cube(1.0f,2.0f,1.0f);
		Cube* right = new Cube(1.0f, 2.0f, 1.0f);

		/*if (colorSwitch) { left->color= right->color = { 255,255,255 }; colorSwitch = false; }
		else { left->color= right->color = { 255,0,0 }; colorSwitch = true; }*/

		App->scene_intro->PrimitiveObjects.PushBack(left);
		App->scene_intro->PrimitiveObjects.PushBack(right);

		switch (direction)
		{
			case RIGHT_RECT: x += 3.0f; left->SetPos(x, y + 1, z+width); right->SetPos(x, y + 1, z-width); break; //Right
			case LEFT_RECT: x -= 3.0f; left->SetPos(x, y + 1, z - width); right->SetPos(x, y + 1, z + width); break; //Left
			case FORWARD_RECT: z += 3.0f; left->SetPos(x - width, y + 1, z); right->SetPos(x + width, y + 1, z); break; //Forward
			case BACKWARD_RECT: z -= 3.0f; left->SetPos(x + width, y + 1, z); right->SetPos(x - width, y + 1, z); break; //Backward
		}
		AddBody(*left, 1000000.0f, WALL);
		AddBody(*right, 1000000.0f, WALL);
	}
}

void ModulePhysics3D::Corner(int width, int x, int y, int z, RoadTypes direction)
{
	//bool colorSwitch = false;
	Cube* lonely = new Cube(1.0f, 2.0f, 1.0f);
	Cube* rectZ = new Cube(1.0f, 2.0f, width);
	Cube* rectX = new Cube(width, 2.0f, 1.0f);

	App->scene_intro->PrimitiveObjects.PushBack(lonely);
	App->scene_intro->PrimitiveObjects.PushBack(rectZ);
	App->scene_intro->PrimitiveObjects.PushBack(rectX);

	/*if (colorSwitch) { lonely->color = { 255,255,255 }; colorSwitch = false; }
	else { lonely->color = { 255,0,0 }; colorSwitch = true; }*/

	switch (direction)
	{
		case TOP_RIGHT_CORNER:
			lonely->SetPos(x + width, y + 1, z);
			rectX->SetPos(x + (width / 2), y + 1, z + width);
			rectZ->SetPos(x, y + 1, z + (width / 2));
			break;
		case TOP_LEFT_CORNER:
			lonely->SetPos(x - width, y + 1, z);
			rectX->SetPos(x - (width / 2), y + 1, z + width);
			rectZ->SetPos(x, y + 1, z + (width / 2));
			break;
		case BOTTOM_RIGHT_CORNER:
			lonely->SetPos(x + width, y + 1, z);
			rectX->SetPos(x + (width / 2), y + 1, z - width);
			rectZ->SetPos(x, y + 1, z - (width / 2));
			break;
		case BOTTOM_LEFT_CORNER:
			lonely->SetPos(x - width, y + 1, z);
			rectX->SetPos(x - (width / 2), y + 1, z - width);
			rectZ->SetPos(x, y + 1, z - (width / 2));
			break;
	}

	AddBody(*lonely, 1000000.0f, WALL);
	AddBody(*rectZ, 1000000.0f, WALL);
	AddBody(*rectX, 1000000.0f, WALL);
}

void ModulePhysics3D::StartZone(int width, int x, int y, int z)
{
	//bool colorSwitch = false;
	Cube* Large = new Cube(width, 2.0f, 1.0f);
	Cube* ShortLeft = new Cube(1.0f, 2.0f, width/2);
	Cube* ShortRight = new Cube(1.0f, 2.0f, width/2);

	App->scene_intro->PrimitiveObjects.PushBack(Large);
	App->scene_intro->PrimitiveObjects.PushBack(ShortLeft);
	App->scene_intro->PrimitiveObjects.PushBack(ShortRight);

	/*if (colorSwitch) { lonely->color = { 255,255,255 }; colorSwitch = false; }
	else { lonely->color = { 255,0,0 }; colorSwitch = true; }*/

	Large->SetPos(x, y + 1, z-1);
	ShortLeft->SetPos(x + (width / 2), y + 1, z+(width/4));
	ShortRight->SetPos(x-(width/2), y + 1,z+(width/4));

	AddBody(*Large, 1000000.0f, WALL);
	AddBody(*ShortLeft, 1000000.0f, WALL);
	AddBody(*ShortRight, 1000000.0f, WALL);
}


void ModulePhysics3D::DiagonalRoad(int length, int width, int x, int y, int z, int direction)
{
	bool colorSwitch = false;
	for (int n = 0; n < length * 2; n++)
	{
		Cube* left = new Cube(1.0f, 2.0f, 1.0f);
		Cube* right = new Cube(1.0f, 2.0f, 1.0f);

		if (colorSwitch) { left->color = right->color = { 255,255,255 }; colorSwitch = false; }
		else { left->color = right->color = { 255,0,0 }; colorSwitch = true; }

		App->scene_intro->PrimitiveObjects.PushBack(left);
		App->scene_intro->PrimitiveObjects.PushBack(right);

		switch (direction)
		{
			case 0: x += 2; z -= 2; left->SetPos(x - width, y + 1, z - width); right->SetPos(x + width, y + 1, z + width); break; //Top-Left
			case 1: x += 2; z += 2; left->SetPos(x + width, y + 1, z - width); right->SetPos(x - width, y + 1, z + width); break; //Bottom-Left
			case 2: z -= 2; x -= 2; left->SetPos(x + width, y + 1, z - width); right->SetPos(x - width, y + 1, z + width); break; //Top-Right
			case 3: z += 2; x -= 2; left->SetPos(x + width, y + 1, z + width); right->SetPos(x - width, y + 1, z - width); break; //Bottom-Right
		}
		AddBody(*left, 1000000.0f, WALL);
		AddBody(*right, 1000000.0f, WALL);
	}
}

void ModulePhysics3D::Ground(int length, int width, int x, int y, int z)
{
	Cube* ground = new Cube(length, 0.05f, width);

	ground->color = Black;

	App->scene_intro->PrimitiveObjects.PushBack(ground);

	ground->SetPos(x,y,z);
	//AddBody(*ground, 10000.0f);
}

void ModulePhysics3D::Enemy(int length, int width, int height, int x, int y, int z)
{
	Cube* enemy = new Cube(length, height, width);

	enemy->color = { 178,0,255 };

	App->scene_intro->PrimitiveObjects.PushBack(enemy);

	enemy->SetPos(x, y, z);

	AddBody(*enemy, 1000000.0f,ENEMY);
}

void ModulePhysics3D::Checkpoint(int x, int y, int z)
{
	Cube* checkpoint = new Cube(0.5, 1.0f, 0.5);

	checkpoint->color = { 0,0,255 }; 

	App->scene_intro->PrimitiveObjects.PushBack(checkpoint);

	checkpoint->SetPos(x, y+0.5f, z);

	AddBody(*checkpoint, 1000000.0f, CHECKPOINT,x,y,z);
}

void ModulePhysics3D::Goal(int x, int y, int z)
{
	Cube* finish = new Cube(15, 6, 1);

	finish->color = { 0,255,0 }; 

	App->scene_intro->PrimitiveObjects.PushBack(finish);

	finish->SetPos(x, y+3, z);

	AddBody(*finish, 100.0f, WIN);
}

// =============================================
void DebugDrawer::drawLine(const btVector3& from, const btVector3& to, const btVector3& color)
{
	line.origin.Set(from.getX(), from.getY(), from.getZ());
	line.destination.Set(to.getX(), to.getY(), to.getZ());
	line.color.Set(color.getX(), color.getY(), color.getZ());
	line.Render();
}

void DebugDrawer::drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color)
{
	point.transform.translate(PointOnB.getX(), PointOnB.getY(), PointOnB.getZ());
	point.color.Set(color.getX(), color.getY(), color.getZ());
	point.Render();
}

void DebugDrawer::reportErrorWarning(const char* warningString)
{
	LOG("Bullet warning: %s", warningString);
}

void DebugDrawer::draw3dText(const btVector3& location, const char* textString)
{
	LOG("Bullet draw text: %s", textString);
}

void DebugDrawer::setDebugMode(int debugMode)
{
	mode = (DebugDrawModes) debugMode;
}

int	 DebugDrawer::getDebugMode() const
{
	return mode;
}
