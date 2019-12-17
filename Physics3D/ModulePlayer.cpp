#include "Globals.h"
#include "Application.h"
#include "ModulePlayer.h"
#include "Primitive.h"
#include "PhysVehicle3D.h"
#include "PhysBody3D.h"

ModulePlayer::ModulePlayer(Application* app, bool start_enabled) : Module(app, start_enabled), vehicle(NULL)
{
	turn = acceleration = brake = 0.0f;
}

ModulePlayer::~ModulePlayer()
{}

// Load assets
bool ModulePlayer::Start()
{
	LOG("Loading player");
	alive = true;
	fxPlayed = false;
	lastCheckpoint = {0,2,0};
	App->audio->LoadFx("Sounds/WinSound.wav");
	App->audio->LoadFx("Sounds/CarCrash.wav");
	VehicleInfo car;

	// Car properties ----------------------------------------
	car.chassis_size.Set(2, 1, 4);
	car.chassis_offset.Set(0, 0.5, 0);
	car.front_chassis_size.Set(3,0.5,1);
	car.front_chassis_offset.Set(0,0.3,2);
	car.rear_chassis_size.Set(3.5,0.3,1);
	car.rear_chassis_offset.Set(0,1,-2);
	car.cabine_radius = 0.8f;
	car.cabine_offset.Set(0,0.9,1);
	car.mass = 500.0f;
	car.suspensionStiffness = 15.0f;
	car.suspensionCompression = 2.0f;
	car.suspensionDamping = 1.0f;
	car.maxSuspensionTravelCm = 100.0f;
	car.frictionSlip = 50.5f;
	car.maxSuspensionForce = 10000.0f;

	// Wheel properties ---------------------------------------
	float connection_height = 1.0f;
	float wheel_radius = 0.6f;
	float wheel_width = 0.5f;
	float suspensionRestLength = 1.2f;

	// Don't change anything below this line ------------------

	float half_width = car.chassis_size.x*0.65f;
	float half_length = car.chassis_size.z*0.5f;
	
	vec3 direction(0,-1,0);
	vec3 axis(-1,0,0);
	
	car.num_wheels = 4;
	car.wheels = new Wheel[4];

	// FRONT-LEFT ------------------------
	car.wheels[0].connection.Set(half_width - 0.3f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[0].direction = direction;
	car.wheels[0].axis = axis;
	car.wheels[0].suspensionRestLength = suspensionRestLength;
	car.wheels[0].radius = wheel_radius;
	car.wheels[0].width = wheel_width;
	car.wheels[0].front = true;
	car.wheels[0].drive = true;
	car.wheels[0].brake = false;
	car.wheels[0].steering = true;

	// FRONT-RIGHT ------------------------
	car.wheels[1].connection.Set(-half_width + 0.3f * wheel_width, connection_height, half_length - wheel_radius);
	car.wheels[1].direction = direction;
	car.wheels[1].axis = axis;
	car.wheels[1].suspensionRestLength = suspensionRestLength;
	car.wheels[1].radius = wheel_radius;
	car.wheels[1].width = wheel_width;
	car.wheels[1].front = true;
	car.wheels[1].drive = true;
	car.wheels[1].brake = false;
	car.wheels[1].steering = true;

	// REAR-LEFT ------------------------
	car.wheels[2].connection.Set(half_width - 0.3f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[2].direction = direction;
	car.wheels[2].axis = axis;
	car.wheels[2].suspensionRestLength = suspensionRestLength;
	car.wheels[2].radius = wheel_radius;
	car.wheels[2].width = wheel_width;
	car.wheels[2].front = false;
	car.wheels[2].drive = false;
	car.wheels[2].brake = true;
	car.wheels[2].steering = false;

	// REAR-RIGHT ------------------------
	car.wheels[3].connection.Set(-half_width + 0.3f * wheel_width, connection_height, -half_length + wheel_radius);
	car.wheels[3].direction = direction;
	car.wheels[3].axis = axis;
	car.wheels[3].suspensionRestLength = suspensionRestLength;
	car.wheels[3].radius = wheel_radius;
	car.wheels[3].width = wheel_width;
	car.wheels[3].front = false;
	car.wheels[3].drive = false;
	car.wheels[3].brake = true;
	car.wheels[3].steering = false;

	/*antena = new Cylinder(0.05f,1.0f);
	antena_body = App->physics->AddBody(*antena,1.0f);
	antena->SetBody(antena_body);
	car.jointOffset = car.chassis_offset;
	antena_anchor = { -1,0,0 };
	car.joint = App->physics->AddBody(Cube(0.1f,0.1f,0.1f),1.0f);
	App->physics->AddConstraintP2P(*antena_body->body, *car.joint->body,antena_anchor, car.chassis_offset);*/

	vehicle = App->physics->AddVehicle(car);
	vehicle->SetPos(0, 2, 0);
	//vehicle->SetPos(60, 3, -170);
	vehicle->collision_listeners.add(this);
	return true;
}

// Unload assets
bool ModulePlayer::CleanUp()
{
	LOG("Unloading player");
	return true;
}

// Update: draw background
update_status ModulePlayer::Update(float dt)
{
	turn = acceleration = brake = 0.0f;
	if(alive)
    {
		if (App->input->GetKey(SDL_SCANCODE_UP) == KEY_REPEAT)
		{
			acceleration = MAX_ACCELERATION*2;
		}

		if (App->input->GetKey(SDL_SCANCODE_LEFT) == KEY_REPEAT)
		{
			if (turn < TURN_DEGREES)
				turn += TURN_DEGREES;
		}

		if (App->input->GetKey(SDL_SCANCODE_RIGHT) == KEY_REPEAT)
		{
			if (turn > -TURN_DEGREES)
				turn -= TURN_DEGREES;
		}

		if (App->input->GetKey(SDL_SCANCODE_DOWN) == KEY_REPEAT)
		{
			acceleration = -MAX_ACCELERATION*2;
		}

		if (App->input->GetKey(SDL_SCANCODE_X) == KEY_REPEAT)
		{
			brake = BRAKE_POWER;
		}
	}

	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		RestartPlayer(lastCheckpoint.x+1, lastCheckpoint.y +2, lastCheckpoint.z+1);
	}

	
	vehicle->ApplyEngineForce(acceleration);
	vehicle->Turn(turn);
	vehicle->Brake(brake);

	vehicle->Render();
	char title[80];
	sprintf_s(title, "%.1f Km/h", vehicle->GetKmh());
	App->window->SetTitle(title);

	return UPDATE_CONTINUE;
}


void ModulePlayer::RestartPlayer(int x,int y, int z)
{	
	//brake = BRAKE_POWER;
	vehicle->SetPos(x, y, z);	
	alive = true;
	fxPlayed = false;
}

void ModulePlayer::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{	
	if (body2->collType == ENEMY)
	{
		alive = false;
		if (!fxPlayed) { App->audio->PlayFx(2, 0); fxPlayed = true; }
	}

	if (body2->collType == WIN)
	{
		alive = false;
		if (!fxPlayed) { App->audio->PlayFx(1, 0); fxPlayed = true; }
	}

	if (body2->collType == CHECKPOINT)
	{
		LOG("Collision checkpoint");
		lastCheckpoint.x = body2->checkpointX;		
		lastCheckpoint.y = body2->checkpointY;
		lastCheckpoint.z = body2->checkpointZ;
	}
}
