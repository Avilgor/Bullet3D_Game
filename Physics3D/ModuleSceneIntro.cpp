#include "Globals.h"
#include "Application.h"
#include "ModuleSceneIntro.h"
#include "Primitive.h"
#include "PhysBody3D.h"
#include "ModuleAudio.h"
#include <ctime>

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

	App->audio->PlayMusic("Sounds/BackgroundMusic.wav",-1.0f);

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
		App->player->lastCheckpoint.x = -3;
		App->player->lastCheckpoint.y = 1;
		App->player->lastCheckpoint.z = -3;
		App->audio->PlayMusic("Sounds/BackgroundMusic.wav", -1.0f);
	}

	//mat4x4 transMatrix;
	for (uint n = 0; n < PrimitiveObjects.Count(); n++)
	{		
		//PrimitiveObjects[n]->body->GetTransform(&transMatrix);
		//PrimitiveObjects[n]->transform = transMatrix;
		PrimitiveObjects[n]->Render();
	}	

	return UPDATE_CONTINUE;
}


void ModuleSceneIntro::BuildMap()
{
	LOG("Building map.");
	//Ground
	App->physics->Ground(500,500,0,0,0);

	//Start box
	App->physics->StartZone(17,0, 0, -7);

	// Circuit
	App->physics->RectRoad(7, 8, 0, 0, 0, FORWARD_RECT);
	App->physics->Corner(15, -8,0, 45, TOP_RIGHT_CORNER);
	App->physics->RectRoad(4, 8, 6, 0, 52, RIGHT_RECT);
	App->physics->Corner(15, 47, 0, 60, BOTTOM_LEFT_CORNER); //Checkpoint location
	App->physics->Corner(15, 32, 0, 62, TOP_RIGHT_CORNER);
	App->physics->RectRoad(10, 8, 46, 0, 70, RIGHT_RECT);
	App->physics->Corner(15, 122, 0, 63, TOP_LEFT_CORNER); //Checkpoint location
	App->physics->RectRoad(3, 8, 115, 0, 63, BACKWARD_RECT);
	App->physics->Corner(15, 123, 0, 43, BOTTOM_LEFT_CORNER);
	App->physics->RectRoad(5, 8, 77, 0, 35, RIGHT_RECT); 
	App->physics->Corner(15, 63, 0, 28, TOP_RIGHT_CORNER); // Checkpoint location
	App->physics->RectRoad(30, 8, 71, 0, 29, BACKWARD_RECT); ////
	App->physics->StartZone(17, 71, 0, -161);
	App->physics->Goal(71,0,-154);

	//Enemies
	srand(time(NULL));
	for (int a = 0; a < 9; a++)
	{
		App->physics->Enemy(1, 1, 2, rand() % (6 - (-6) + 1) - 6, 1, rand() % (40 - 5 + 1) + 5);
	}
	for (int a = 0; a < 8; a++)
	{
		App->physics->Enemy(1, 1, 2, rand() % (35 - 6 + 1) + 6, 1, rand() % (58 - 43 + 1) + 43);
	}
	for (int a = 0; a < 12; a++)
	{
		App->physics->Enemy(1, 1, 2, rand() % (105 - 40 + 1) + 40, 1, rand() % (75 - 63 + 1) + 65);
	}
	for (int a = 0; a < 8; a++)
	{
		App->physics->Enemy(1, 1, 2, rand() % (120 - 110 + 1) + 110, 1, rand() % (62 - 40 + 1) + 40);
	}
	for (int a = 0; a < 8; a++)
	{
		App->physics->Enemy(1, 1, 2, rand() % (120 - 73 + 1) + 73, 1, rand() % (40 - 29 + 1) + 29);
	}
	for (int a = 0; a < 42; a++)
	{
		App->physics->Enemy(1, 1, 2, rand() % (78 - 65 + 1) + 65, 1, rand() % (29 - (-150) + 1) - 150);
	}

	//Checkpoints
	App->physics->Checkpoint(40, 0, 60);
	App->physics->Checkpoint(115, 0, 68);
	App->physics->Checkpoint(68, 0, 30);
}