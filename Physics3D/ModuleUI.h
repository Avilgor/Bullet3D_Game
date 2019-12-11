/*#ifndef _MODULEUI_H
#define _MODULEUI_H

#include "Module.h"
#include "ModuleFont.h"
#include "Globals.h"

#include "SDL_mixer/include/SDL_mixer.h"

#define SECOND 1000

struct SDL_Texture;

class ModuleUI : public Module
{
public:
	ModuleUI();
	~ModuleUI();

	bool Start();
	update_status Update();
	bool CleanUp();

	void Counter();
	void Result();
	void showScreen(int x, int y, int num);

public:

	SDL_Texture* graphics1 = nullptr;
	SDL_Texture* graphics2 = nullptr;


	int round1 = 0;

	int timer = 0;

	int font_id;
	int font_Rounds;
	int font2;
	int time;
	int Counter1;
	int Counter2;
	int timenow;

private:
	int six, five, four, three, two, one;

};

#endif // !_MODULEUI_H
*/