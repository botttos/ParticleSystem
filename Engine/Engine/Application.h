#ifndef _APPLICATION_H_
#define _APPLICATION_H_

#include <list>
#include "Globals.h"
#include "Timer.h"
#include "PerfTimer.h"

#define GRAPH_ARRAY_SIZE 30

using namespace std;

class Module;
class ModuleWindow;
class FileSystem;
class ModuleInput;
class InputManager;
class ModuleAudio;
class ModuleRenderer3D;
class ModuleCamera3D;
class ModulePhysics3D;
class ModuleImgui;
class ModuleConsole;
class ModuleHardware;
class GeometryManager;
class Profiler;
class Scene;
enum MODULE_ID;

enum APP_CONTEXT
{
	UNKNOWN_CONTEXT = 0,
	BUILD_CONTEXT,
	PLAY_CONTEXT,
	IN_GAME_CONTEXT,
	PAUSE_CONTEXT,
	MAIN_MENU_CONTEXT
};

class Application
{
public:

	Application();
	~Application();

public:

	bool			Awake();
	bool			Init();
	void			PrepareUpdate();
	update_status	Update();
	void			FinishUpdate();
	bool			CleanUp();

public:

	ModuleWindow*		window = nullptr;
	FileSystem*			fs = nullptr;
	ModuleInput*		input = nullptr;
	InputManager*		input_manager = nullptr;
	ModuleAudio*		audio = nullptr;
	ModuleRenderer3D*	renderer3D = nullptr;
	ModuleCamera3D*		camera = nullptr;
	ModulePhysics3D*	physics = nullptr;
	ModuleImgui*		imgui = nullptr;
	ModuleConsole*		console = nullptr;
	ModuleHardware*		hard = nullptr;
	GeometryManager*	geometry = nullptr;
	Scene*				scene = nullptr;

	//Tools 
	Profiler*			profiler = nullptr;

	APP_CONTEXT			app_context = UNKNOWN_CONTEXT;

private:

	PerfTimer		prof_timer;
	PerfTimer		m_prof_timer;
	Timer			ms_timer;
	uint64			frame_count = 0;
	Timer			startup_time;
	Timer			frame_time;
	Timer			last_sec_frame_time;
	uint32			last_sec_frame_count = 0;
	uint32			prev_last_sec_frame_count = 0;
	float			dt = 0.0f;
	int				capped_ms = -1;
	int				max_fps = 0;

	list<Module*>	list_modules;
	bool			want_to_quit = false;

	float			fps_array[GRAPH_ARRAY_SIZE];
	float			miliseconds_array[GRAPH_ARRAY_SIZE];
	bool			config_opened = false;

public:

	bool			show_config_window = false;

	std::string		app_name;
	std::string		organization;

public:

	// Used in data loading
	APP_CONTEXT	StrToAppContext(const char* str) const;
	const char*	AppContextToStr(APP_CONTEXT app_context) const;
	MODULE_ID	StrToModuleID(const char* str)const;

	void SetQuit();
	void RequestBrowser(const char* link);
	void ShowConfiguration();
	void BlitConfigWindow();

private:

	void AddModule(Module* mod);

};

extern Application* App;

#endif // !_APPLICATION_H_


