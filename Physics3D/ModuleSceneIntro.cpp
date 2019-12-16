#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "Primitive.h"
#include "PhysBody3D.h"
#include "ModuleAudio.h"

ModuleSceneIntro::ModuleSceneIntro(Application* app, bool start_enabled) : Module(app, start_enabled)
{
}

ModuleSceneIntro::~ModuleSceneIntro()
{}

// Load assets
bool ModuleSceneIntro::Start()
{
	LOG("Loading Intro assets");
	bool ret = true;

	//App->audio->PlayMusic("path",-1.0f);

	App->camera->Move(vec3(1.0f, 1.0f, 0.0f));
	App->camera->LookAt(vec3(0, 0, 0));
	Checkpoint = (0.0f, 3.0f, 0.0f);
	BuildMap();

	return ret;
}

// Load assets
bool ModuleSceneIntro::CleanUp()
{
	LOG("Unloading Intro scene");

	return true;
}

// Update
update_status ModuleSceneIntro::Update(float dt)
{
	Plane p(0, 1, 0, 0);
	p.axis = true;
	p.Render();


	//Restart Level
	if (App->input->GetKey(SDL_SCANCODE_R) == KEY_DOWN)
	{
		App->player->RestartPlayer(0,3,0);
		Checkpoint = (0, 3, 0);
	}

	//Restart to checkpoint
	if (App->input->GetKey(SDL_SCANCODE_SPACE) == KEY_DOWN)
	{
		App->player->RestartPlayer(Checkpoint.x, Checkpoint.y+2, Checkpoint.z);
	}

	for (uint n = 0; n < PrimitiveObjects.Count(); n++)
		PrimitiveObjects[n]->Render();

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{
	LOG("Collision detected");
	if (body1->collType == CAR && body2->collType == ENEMY)
	{
		App->player->alive = false;
	}

	if (body1->collType == CAR && body2->collType == CHECKPOINT)
	{
		LOG("Checkpoint collision");
		//Checkpoint = 
	}
}

void ModuleSceneIntro::BuildMap()
{
	LOG("Building map.");
	//Ground
	//App->physics->Ground(100,100,0,-0.2f,0);

	// First rect
	App->physics->RectRoad(5, 10, 0, 0, 0, 0);
	App->physics->RectRoad(5, 10, 0, 0, 0, 1);
	App->physics->RectRoad(5, 10, 0, 0, 0, 2);
	App->physics->RectRoad(5, 10, 0, 0, 0, 3);

	// First diagonal
	App->physics->DiagonalRoad(5, 5, 0, 0, 0, 0);
	App->physics->DiagonalRoad(5, 5, 0, 0, 0, 1);
	App->physics->DiagonalRoad(5, 5, 0, 0, 0, 2);
	App->physics->DiagonalRoad(5, 5, 0, 0, 0, 3);

	// Second rect
	//App->physics->RectRoad(5, 10, 0, 10, 0);

	// Second diagonal
	//App->physics->DiagonalRoad(5,15,0,15,0);

	//Enemies
	App->physics->Enemy(1, 1, 2, 3, 1, 3);

	//Checkpoints
	App->physics->Checkpoint(-3, 1, -3);
}