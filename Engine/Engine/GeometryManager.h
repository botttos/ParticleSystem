#ifndef _GEOMETRY_MANAGER_H
#define _GEOMETRY_MANAGER_H

#include "Module.h"

#include "Primitive_.h"
#include "Cube_.h"

class GeometryManager : public Module
{
public:

	GeometryManager(const char* _name, MODULE_ID _id, bool _config_menu, bool _enabled = true);
	~GeometryManager();

public:

	bool Draw();
	bool CleanUp();

private:

	std::list<Primitive_*> geometry_list;

public:

	//Geometry factory ------
	Primitive_* CreatePrimitive(PRIMITIVE_TYPE type, ...);

};

#endif // !_GEOMETRY_MANAGER_H
