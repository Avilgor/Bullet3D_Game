#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "Primitive.h"
#include "PhysBody3D.h"

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
		App->player->RestartPlayer();
	}

	for (uint n = 0; n < PrimitiveObjects.Count(); n++)
		PrimitiveObjects[n]->Render();

	return UPDATE_CONTINUE;
}

void ModuleSceneIntro::OnCollision(PhysBody3D* body1, PhysBody3D* body2)
{

}

void ModuleSceneIntro::BuildMap()
{
	LOG("Building map.");
	
	// First rect
	App->physics->RectRoad(5,10,0, 0, 0, 2);
	//App->physics->RectRoad(-5, 1, 0, 10, 2, 3);

	// First diagonal
	//App->physics->DiagonalRoad(5, 5, 0, 5, 2);
	//App->physics->DiagonalRoad(-5, 1, 20, 9,  1.5f, 3, 1);

	// Second rect
	//App->physics->RectRoad(5, 10, 0, 10, 0);
	//App->physics->RectRoad(-21, 1, 28, 2,  2, 0);

	// Second diagonal
	//App->physics->DiagonalRoad(5,15,0,15,0);
	//App->physics->DiagonalRoad(-23, 1, 28, 5, vec3(1, 2, 1), 1.5f, 2, 1);
	

}