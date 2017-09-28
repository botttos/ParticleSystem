#include "Globals.h"
#include "Application.h"
#include "ModuleRenderer3D.h"
#include "Glew/include/glew.h"
#include "SDL/include/SDL_opengl.h"
#include <gl/GL.h>
#include <gl/GLU.h>
#include "ModuleWindow.h"
#include "ModuleCamera3D.h"
#include "FileSystem.h"
#include "ModuleAudio.h"
#include "ModuleImgui.h"

#pragma comment (lib, "glu32.lib")    /* link OpenGL Utility lib     */
#pragma comment (lib, "opengl32.lib") /* link Microsoft OpenGL lib   */
#pragma comment (lib, "Engine/Glew/lib/Win32/glew32.lib")

// Constructors =================================
ModuleRenderer3D::ModuleRenderer3D(const char* _name, MODULE_ID _id, bool _config_menu, bool _enabled) : Module(_name, _id, _config_menu, _enabled)
{

}

// Destructors ==================================
ModuleRenderer3D::~ModuleRenderer3D()
{

}

// Game Loop ====================================
bool ModuleRenderer3D::Awake(const JSON_Object * data_root)
{
	vsync = json_object_get_boolean(data_root, "vsync");
	
	depth_test = json_object_get_boolean(data_root, "depth_test");
	
	cull_face = json_object_get_boolean(data_root, "cull_face");
	
	texture_2d = json_object_get_boolean(data_root, "texture_2d");
	
	dither = json_object_get_boolean(data_root, "dither");

	wireframe = json_object_get_boolean(data_root, "wireframe");

	lighting = json_object_get_boolean(data_root, "lighting");
	JSON_Array* lighting_color_array = json_object_get_array(data_root, "lighting_color");
	lighting_color[0] = json_array_get_number(lighting_color_array, 0);
	lighting_color[1] = json_array_get_number(lighting_color_array, 1);
	lighting_color[2] = json_array_get_number(lighting_color_array, 2);
	lighting_color[3] = json_array_get_number(lighting_color_array, 3);
	
	material_color = json_object_get_boolean(data_root, "material_color");
	JSON_Array* material_ambient_array = json_object_get_array(data_root, "material_ambient");
	material_ambient[0] = json_array_get_number(material_ambient_array, 0);
	material_ambient[1] = json_array_get_number(material_ambient_array, 1);
	material_ambient[2] = json_array_get_number(material_ambient_array, 2);
	material_ambient[3] = json_array_get_number(material_ambient_array, 3);
	JSON_Array* material_diffuse_array = json_object_get_array(data_root, "material_diffuse");
	material_diffuse[0] = json_array_get_number(material_diffuse_array, 0);
	material_diffuse[1] = json_array_get_number(material_diffuse_array, 1);
	material_diffuse[2] = json_array_get_number(material_diffuse_array, 2);
	material_diffuse[3] = json_array_get_number(material_diffuse_array, 3);
	
	fog = json_object_get_boolean(data_root, "fog");
	fog_density = json_object_get_number(data_root, "fog_density");
	JSON_Array* fog_color_array = json_object_get_array(data_root, "fog_color");
	fog_color[0] = json_array_get_number(fog_color_array, 0);
	fog_color[1] = json_array_get_number(fog_color_array, 1);
	fog_color[2] = json_array_get_number(fog_color_array, 2);
	fog_color[3] = json_array_get_number(fog_color_array, 3);

	custom_clear = json_object_get_boolean(data_root, "custom_clear");
	JSON_Array* clear_color_array = json_object_get_array(data_root, "clear_color");
	clear_color[0] = json_array_get_number(clear_color_array, 0);
	clear_color[1] = json_array_get_number(clear_color_array, 1);
	clear_color[2] = json_array_get_number(clear_color_array, 2);
	clear_color[3] = json_array_get_number(clear_color_array, 3);
	clear_depth = json_object_get_number(data_root, "clear_depth");
	
	config_menu = true;

	return true;
}

// Called before render is available
bool ModuleRenderer3D::Init()
{
	LOG("Creating 3D Renderer context");
	bool ret = true;
	
	
	//OPENGL initialization
	//Create context
	context = SDL_GL_CreateContext(App->window->window);
	if(context == NULL)
	{
		LOG("[error] OpenGL context could not be created! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	GLenum glew_error = glewInit();
	if (glew_error != GL_NO_ERROR)
	{
		LOG("[error]Error initializing GLew! %s\n", glewGetErrorString(glew_error));
	}
	else
	{
		LOG("Using GLew: %s", glewGetString(GLEW_VERSION));
	}

	if(ret == true)
	{
		//Use Vsync
		if (vsync && SDL_GL_SetSwapInterval(1) < 0)
		{
			LOG("[error] Unable to set VSync! SDL Error: %s\n", SDL_GetError());
		}
		else if(!vsync)
		{
			SDL_GL_SetSwapInterval(0);
		}

		//Initialize Projection Matrix
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		//Check for error
		GLenum error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("[error] Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		//Initialize Modelview Matrix
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//Check for error
		error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("[error] Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}
		
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
				
		//Initialize clear color
		if (custom_clear)
		{
			glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
			glClearDepth(clear_depth);
		}
		else
		{
			glClearColor(0.0, 0.0, 0.0, 0.0);
			glClearDepth(1.0f);
		}

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		//Enable OpenGL Options
		if (depth_test)
		{
			glEnable(GL_DEPTH_TEST);
		}

		if (cull_face)
		{
			glEnable(GL_CULL_FACE);
		}

		if (texture_2d)
		{
			glEnable(GL_TEXTURE_2D);
		}

		if (dither)
		{
			glEnable(GL_DITHER);
		}

		//Initialize lighting states
		if (lighting)glEnable(GL_LIGHTING);
		GLfloat l_color[] = { lighting_color[0], lighting_color[1], lighting_color[2], lighting_color[3] };
		glLightModelfv(GL_EMISSION, l_color);

		//Initialize material states
		if (material_color)
		{
			glEnable(GL_COLOR_MATERIAL);
		}
		GLfloat m_ambient[] = { material_ambient[0], material_ambient[1], material_ambient[2], material_ambient[3] };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_ambient);
		GLfloat m_diffuse[] = { material_diffuse[0], material_diffuse[1], material_diffuse[2], material_diffuse[3] };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diffuse);
		
		//Initialize fog states
		if (fog)
		{
			glEnable(GL_FOG);
		}
		const GLfloat f_color[4] = { fog_color[0], fog_color[1], fog_color[2], fog_color[3] };
		glFogfv(GL_FOG_COLOR, f_color);
		glFogf(GL_FOG_DENSITY, fog_density);
		
		//Maybe we can ignore this?
		//Check for error
		/*error = glGetError();
		if(error != GL_NO_ERROR)
		{
			LOG("[error] Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}*/
		
		//Initialize default light
		lights[0].ref = GL_LIGHT0;
		lights[0].ambient.Set(0.25f, 0.25f, 0.25f, 1.0f);
		lights[0].diffuse.Set(0.75f, 0.75f, 0.75f, 1.0f);
		lights[0].SetPos(0.0f, 0.0f, 2.5f);
		lights[0].Init();
			
		lights[0].Active(true);
	
	}

	// Projection matrix for
	OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);

	return ret;
}

bool ModuleRenderer3D::Start()
{
	/*
	//Generate a cube & triangulate it
	geolib_cube = new math::AABB({ 0,0,0 }, { 1,1,1 });
	math::float3 vertex[36];
	geolib_cube->Triangulate(1, 1, 1, vertex, NULL, NULL, true);

	//Allocate a buffer for the cube data
	glGenBuffers(1, &geolib_cube_id);
	glBindBuffer(GL_ARRAY_BUFFER, geolib_cube_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36 * 3, vertex, GL_STATIC_DRAW);
	glDisableClientState(GL_VERTEX_ARRAY);
	*/
	
	/*
	//Generate a cube & collect the vertex
	geolib_cube = new math::AABB({ 0,0,0 }, { 1,1,1 });
	math::float3 vertex[8];
	geolib_cube->GetCornerPoints(vertex);
	uint indices[36] = {	0,1,2, 2,3,0,   // 36 of indices
							0,3,4, 4,5,0,
							0,5,6, 6,1,0,
							1,6,7, 7,2,1,
							7,4,3, 3,2,7,
							4,7,6, 6,5,4 };

	//Vertex buffer
	glGenBuffers(1, (GLuint*) &(opt_cube_vertex_id));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, opt_cube_vertex_id);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float) * 8, vertex, GL_STATIC_DRAW);
	//Index buffer
	glGenBuffers(1, (GLuint*) &(opt_cube_index_id));
	glBindBuffer(GL_ARRAY_BUFFER, opt_cube_index_id);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uint) * 36 * 3, indices, GL_STATIC_DRAW);

	glDisableClientState(GL_VERTEX_ARRAY);	
	*/
	return true;
}

// PreUpdate: clear buffer
update_status ModuleRenderer3D::PreUpdate(float dt)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(App->camera->GetViewMatrix());

	// light 0 on cam pos
	lights[0].SetPos(App->camera->position.x, App->camera->position.y, App->camera->position.z);

	for(uint i = 0; i < MAX_LIGHTS; ++i)
		lights[i].Render();

	return UPDATE_CONTINUE;
}

update_status ModuleRenderer3D::Update(float dt)
{
	//Draw a simple line
	/*
	glLineWidth(2.0f);
	glBegin(GL_LINES);
	glVertex3f(0.f, 0.f, 0.f);
	glVertex3f(0.f, 10.f, 0.f);
	glEnd();
	glLineWidth(1.0f);
	*/

	GLfloat v0[3] = { 1,1,0 };
	GLfloat v1[3] = { 0,1,0 };
	GLfloat v2[3] = { 0,0,0 };
	GLfloat v3[3] = { 1,0,0 };
	GLfloat v4[3] = { 1,0,1 };
	GLfloat v5[3] = { 1,1,1 };
	GLfloat v6[3] = { 0,1,1 };
	GLfloat v7[3] = { 0,0,1 };

	//Draw a simple cube with triangles in direct mode
	/*
	glBegin(GL_TRIANGLES);  // draw a cube with 12 triangles
	// front face =================
	glVertex3fv(v2);
	glVertex3fv(v1);
	glVertex3fv(v0);

	glVertex3fv(v0);
	glVertex3fv(v3);
	glVertex3fv(v2);

	// back face ==================
	glVertex3fv(v5);
	glVertex3fv(v6);
	glVertex3fv(v7);

	glVertex3fv(v7);
	glVertex3fv(v4);
	glVertex3fv(v5);

	// right face =================
	glVertex3fv(v4);
	glVertex3fv(v3);
	glVertex3fv(v0);

	glVertex3fv(v0);
	glVertex3fv(v5);
	glVertex3fv(v4);

	// left face ==================
	glVertex3fv(v1);
	glVertex3fv(v2);
	glVertex3fv(v7);

	glVertex3fv(v7);
	glVertex3fv(v6);
	glVertex3fv(v1);

	// top face ===================
	glVertex3fv(v6);
	glVertex3fv(v5);
	glVertex3fv(v0);

	glVertex3fv(v0);
	glVertex3fv(v1);
	glVertex3fv(v6);

	// bottom face ================
	glVertex3fv(v2);
	glVertex3fv(v3);
	glVertex3fv(v4);

	glVertex3fv(v4);
	glVertex3fv(v7);
	glVertex3fv(v2);

	glEnd();
	*/

	/*
	glEnableClientState(GL_VERTEX_ARRAY);
	glBindBuffer(GL_ARRAY_BUFFER, geolib_cube_id);
	glVertexPointer(3, GL_FLOAT, 0, NULL);
	glDrawArrays(GL_TRIANGLES, 0, 36 * 3);
	glDisableClientState(GL_VERTEX_ARRAY);
	*/

	/*
	GLubyte indices[] = {	0,1,2, 2,3,0,   // 36 of indices
							0,3,4, 4,5,0,
							0,5,6, 6,1,0,
							1,6,7, 7,2,1,
							7,4,3, 3,2,7,
							4,7,6, 6,5,4 };

	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(3, GL_FLOAT, 0, vertex);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->opt_cube_index_id);
	glDrawElements(GL_TRIANGLES,36, GL_UNSIGNED_BYTE, indices);
	glDisableClientState(GL_VERTEX_ARRAY);
	*/

	return update_status::UPDATE_CONTINUE;

}

// PostUpdate present buffer to screen
update_status ModuleRenderer3D::PostUpdate(float dt)
{
	DisableGLRenderFlags();	

	// Rendering GUI
	App->imgui->RenderUI();

	EnableGLRenderFlags();

	SDL_GL_SwapWindow(App->window->window);

	return UPDATE_CONTINUE;
}

// Called before quitting
bool ModuleRenderer3D::CleanUp()
{
	LOG("Destroying 3D Renderer");

	SDL_GL_DeleteContext(context);

	return true;
}

void ModuleRenderer3D::BlitConfigInfo()
{
	ImGui::Text("GLew Version: ");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(1.0f, 0.64f, 0.0f, 1.0f), "%s", glewGetString(GLEW_VERSION));
	ImGui::Separator();

	//Vsync check box
	if (ImGui::Checkbox("VSync", &vsync))
	{
		if(vsync)
		{
			if (SDL_GL_SetSwapInterval(1) < 0)
			{
				LOG("[error] Unable to set VSync! SDL Error: %s\n", SDL_GetError());
			}
		}
		else SDL_GL_SetSwapInterval(0);
		App->audio->PlayFxForInput(CHECKBOX_FX);
	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Turn ON/OFF VSync.");
	
	//Depth Test check box
	if (ImGui::Checkbox("Depht Test", &depth_test))
	{
		App->audio->PlayFxForInput(CHECKBOX_FX);

		if (depth_test)
		{
			glEnable(GL_DEPTH_TEST);
		}
		else
		{
			glDisable(GL_DEPTH_TEST);
		}
	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Turn ON/OFF the depth pixel painting.");

	//Cull Test check box
	if (ImGui::Checkbox("Cull Test", &cull_face))
	{
		App->audio->PlayFxForInput(CHECKBOX_FX);
		if (cull_face)
		{
			glEnable(GL_CULL_FACE);
		}
		else
		{
			glDisable(GL_CULL_FACE);
		}
	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Turn ON/OFF the triangle culling.");

	//Texture 2D check box
	if (ImGui::Checkbox("Texture 2D", &texture_2d))
	{
		App->audio->PlayFxForInput(CHECKBOX_FX);
		if (texture_2d)
		{
			glEnable(GL_TEXTURE_2D);
		}
		else
		{
			glDisable(GL_TEXTURE_2D);
		}
	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Allows to texture in 2D");

	//Dither check box
	if (ImGui::Checkbox("Dither", &dither))
	{
		App->audio->PlayFxForInput(CHECKBOX_FX);
		if (dither)
		{
			glEnable(GL_DITHER);
		}
		else
		{
			glDisable(GL_DITHER);
		}
	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Dither color components before they are written to the color buffer.");
	ImGui::Separator();


	//Lighting data ---------
	if (ImGui::Checkbox("Lighting", &lighting))
	{
		App->audio->PlayFxForInput(CHECKBOX_FX);
		if (lighting)
		{
			glEnable(GL_LIGHTING);
		}
		else
		{
			glDisable(GL_LIGHTING);
		}

	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Turn ON/OFF the lights");
	if (lighting)
	{
		if (ImGui::DragFloat4("Lighting Color", lighting_color, 0.05, 0.0, 1.0, "%.2f"))
		{
			GLfloat LightModelAmbient[] = { lighting_color[0], lighting_color[1], lighting_color[2],lighting_color[3] };
			glLightModelfv(GL_LIGHT_MODEL_AMBIENT, LightModelAmbient);
		}
	}
	// ----------------------

	ImGui::Separator();
	
	//Material Data ---------
	if (ImGui::Checkbox("Color Material", &material_color))
	{
		App->audio->PlayFxForInput(CHECKBOX_FX);
		if (material_color)
		{
			glEnable(GL_COLOR_MATERIAL);
		}
		else
		{
			glDisable(GL_COLOR_MATERIAL);
		}
	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Allows to have one or more material parameters tracking the current color");
	if (material_color)
	{
		if (ImGui::DragFloat4("Material Ambient", material_ambient, 0.05, 0.0, 1.0, "%.2f"))
		{
			GLfloat m_ambient[] = { material_ambient[0], material_ambient[1], material_ambient[2], material_ambient[3] };
			glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, m_ambient);
		}
		if (ImGui::DragFloat4("Material Diffuse", material_diffuse, 0.05, 0.0, 1.0, "%.2f"))
		{
			GLfloat m_diffuse[] = { material_diffuse[0], material_diffuse[1], material_diffuse[2], material_diffuse[3] };
			glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, m_diffuse);
		}
	}
	// ----------------------

	ImGui::Separator();

	//Fog Data --------------
	if (ImGui::Checkbox("Fog", &fog))
	{
		App->audio->PlayFxForInput(CHECKBOX_FX);
		if (fog)
		{
			glEnable(GL_FOG);
			glFogf(GL_FOG_DENSITY, fog_density);
		}
		else
		{
			glDisable(GL_FOG);
		}
	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Turns ON/OFF the Fog");

	if (fog)
	{
		if (ImGui::DragFloat4("Fog Color", fog_color, 0.05, 0.0, 1.0, "%.2f"))
		{
			GLfloat color[] = { fog_color[0], fog_color[1], fog_color[2], fog_color[3] };
			glFogfv(GL_FOG_COLOR, color);
		}
		if (ImGui::SliderFloat("Density", &fog_density, 0.0f, 1.0f))
		{
			App->audio->PlayFxForInput(SLICE_TICK_FX);
			glFogf(GL_FOG_DENSITY, fog_density);
		}
	}
	// ----------------------

	ImGui::Separator();

	//Wireframe -------------

	if (ImGui::Checkbox("WireFrame", &wireframe));
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Turns ON/OFF the WireFrame mode");

	if (wireframe)
	{
		if (ImGui::Checkbox("Front Mode", &front_wireframe));
	}
	

	//-----------------------

	ImGui::Separator();

	//Clear Data ------------
	if(ImGui::Checkbox("Custom Clear", &custom_clear))
	{
		App->audio->PlayFxForInput(SLICE_TICK_FX);
		if (!custom_clear)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClearDepth(1.0f);
		}
		else
		{
			glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
			glClearDepth(clear_depth);
		}
	}
	ImGui::SameLine(); ImGui::MyShowHelpMarker("(?)", "Specify the red, green, blue, and alpha values used when the color buffers are cleared");
	if (custom_clear)
	{
		if (ImGui::DragFloat4("Clear Color", clear_color, 0.05, 0.0, 1.0, "%.2f"))
		{
			glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
		}
		if (ImGui::DragFloat("Clear Depth", &clear_depth, 0.05, 0.0, 1.0, "%.2f"))
		{
			glClearDepth(clear_depth);
		}
	}
	// ----------------------
}

void ModuleRenderer3D::SaveConfigInfo(JSON_Object * data_root)
{
	json_object_set_boolean(data_root, "vsync", vsync);
	json_object_set_boolean(data_root, "depth_test", depth_test);
	json_object_set_boolean(data_root, "cull_face", cull_face);
	json_object_set_boolean(data_root, "texture_2d", texture_2d);
	json_object_set_boolean(data_root, "dither", dither);
	// Lighting data
	json_object_set_boolean(data_root, "lighting", lighting);
	json_array_t*_array = json_object_get_array(data_root, "lighting_color");
	json_array_replace_number(_array, 0, lighting_color[0]);
	json_array_replace_number(_array, 1, lighting_color[1]);
	json_array_replace_number(_array, 2, lighting_color[2]);
	json_array_replace_number(_array, 3, lighting_color[3]);
	// Material data
	json_object_set_boolean(data_root, "material_color", material_color);
	_array = json_object_get_array(data_root, "material_ambient");
	json_array_replace_number(_array, 0, material_ambient[0]);
	json_array_replace_number(_array, 1, material_ambient[1]);
	json_array_replace_number(_array, 2, material_ambient[2]);
	json_array_replace_number(_array, 3, material_ambient[3]);
	_array = json_object_get_array(data_root, "material_diffuse");
	json_array_replace_number(_array, 0, material_diffuse[0]);
	json_array_replace_number(_array, 1, material_diffuse[1]);
	json_array_replace_number(_array, 2, material_diffuse[2]);
	json_array_replace_number(_array, 3, material_diffuse[3]);
	//Fog data
	json_object_set_boolean(data_root, "fog", fog);
	json_object_set_number(data_root, "fog_density", fog_density);
	_array = json_object_get_array(data_root, "fog_color");
	json_array_replace_number(_array, 0, fog_color[0]);
	json_array_replace_number(_array, 1, fog_color[1]);
	json_array_replace_number(_array, 2, fog_color[2]);
	json_array_replace_number(_array, 3, fog_color[3]);
	//Clear data
	json_object_set_boolean(data_root, "custom_clear", custom_clear);
	_array = json_object_get_array(data_root, "clear_color");
	json_array_replace_number(_array, 0, clear_color[0]);
	json_array_replace_number(_array, 1, clear_color[1]);
	json_array_replace_number(_array, 2, clear_color[2]);
	json_array_replace_number(_array, 3, clear_color[3]);
	json_object_set_number(data_root, "clear_depth", clear_depth);
}

// Functionality ================================
void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ProjectionMatrix = perspective(60.0f, (float)width / (float)height, 0.125f, 512.0f);
	glLoadMatrixf(&ProjectionMatrix);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

bool ModuleRenderer3D::GetWireframe() const
{
	return wireframe;
}

bool ModuleRenderer3D::GetWireframeFront() const
{
	return front_wireframe;
}

void ModuleRenderer3D::DisableGLRenderFlags()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glDisable(GL_LIGHTING);
}

void ModuleRenderer3D::EnableGLRenderFlags()
{
	if (lighting)glEnable(GL_LIGHTING);
}
