#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <list>
#include "Globals.h"
#include "Timer.h"

using namespace std;

class Module;
class ModuleWindow;
class ModuleInput;
class ModuleAudio;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModulePhysics3D;
class ModuleImgui;
class ModuleConsole;
class Scene;

enum APP_CONTEXT
{
	BUILD_CONTEXT = 0,
	PLAY_CONTEXT
};

class Application
{
public:

	Application();
	~Application();

public:

	bool			Init();
	void			PrepareUpdate();
	update_status	Update();
	void			FinishUpdate();
	bool			CleanUp();

public:

	ModuleWindow*		window = nullptr;
	ModuleInput*		input = nullptr;
	ModuleAudio*		audio = nullptr;
	ModuleRenderer3D*	renderer3D = nullptr;
	ModuleCamera3D*		camera = nullptr;
	ModulePhysics3D*	physics = nullptr;
	ModuleImgui*		imgui = nullptr;
	ModuleConsole*		console = nullptr;
	Scene*				scene = nullptr;

private:

	Timer			ms_timer;
	float			dt = 0.0f;
	list<Module*>	list_modules;
	bool			want_to_quit = false;

public:

	void SetQuit();

private:

	void AddModule(Module* mod);

};

extern Application* _Application;

#endif // !_APPLICATION_H_


