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
		App->player->lastCheckpoint.x = 0;
		App->player->lastCheckpoint.x = 3;
		App->player->lastCheckpoint.x = 0;
	}

	for (uint n = 0; n < PrimitiveObjects.Count(); n++)
		PrimitiveObjects[n]->Render();

	return UPDATE_CONTINUE;
}


void ModuleSceneIntro::BuildMap()
{
	LOG("Building map.");
	//Ground
	App->physics->Ground(500,500,0,0,0);

	// Circuit
	//App->physics->RectRoad(7, 8, 0, 0, 0, FORWARD_RECT);
	//App->physics->DiagonalRoad(5, 5, -3, 0, 40, 3);
	//App->physics->DiagonalRoad(3, 5, 3, 0, 10, 1);
	//App->physics->RectRoad(4, 10, 0, 0, 13, 1);

	App->physics->Corner(10, 0, 0, 0, BOTTOM_RIGHT_CORNER);
	//Enemies
	App->physics->Enemy(1, 1, 2, 3, 1, 3);

	//Checkpoints
	App->physics->Checkpoint(-3, 1, -3);
}