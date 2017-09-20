#ifndef _MODULE_PHYSICS_3D_H_
#define _MODULE_PHYSICS_3D_H_

#include "Module.h"
#include "Globals.h"
#include "Primitive.h"

#include "Bullet/include/btBulletDynamicsCommon.h"
#include "MathGeoLib/MathGeoLib.h"

// Recommended scale is 1.0f == 1 meter, no less than 0.2 objects
#define GRAVITY btVector3(0.0f, -20.0f, 0.0f) 

class DebugDrawer;
struct PhysBody3D;
struct PhysVehicle3D;
struct VehicleInfo;

enum BODY_MASK
{
	NOTHING = 1,
	CAR = 2,
	MAP = 4,
	SENSOR = 8
};

class ModulePhysics3D : public Module
{
public:

	ModulePhysics3D(bool start_enabled = true);
	~ModulePhysics3D();

public:

	bool			Init() final;
	bool			Start() final;
	update_status	PreUpdate(float dt) final;
	update_status	Update(float dt) final;
	update_status	PostUpdate(float dt) final;
	bool			CleanUp() final;

private:

	bool debug = false;

	btDefaultCollisionConfiguration*		collision_conf = nullptr;
	btCollisionDispatcher*					dispatcher = nullptr;
	btBroadphaseInterface*					broad_phase = nullptr;
	btSequentialImpulseConstraintSolver*	solver = nullptr;
	btDiscreteDynamicsWorld*				world = nullptr;
	btDefaultVehicleRaycaster*				vehicle_raycaster = nullptr;
	DebugDrawer*							debug_draw = nullptr;

	list<btCollisionShape*>			shapes;
	list<PhysBody3D*>				bodies;
	list<btTypedConstraint*>		constrains;
	list<btDefaultMotionState*>		motions;

public:

	//Methods to add bodies to the world
	PhysBody3D* AddBody(const _Primitive* primitive, enum OBJECT_TYPE object_type, float mass = 1.0f, BODY_MASK I_am = NOTHING);/*Old one*/
	
	math::Sphere* CreateSphere(math::float3 position, float rad);


	//Methods for constrains
	btPoint2PointConstraint*	Add_P2P_Constraint(btRigidBody& rbA, btRigidBody& rbB, const btVector3& pivotInA, const btVector3& pivotInB);
	btHingeConstraint*			Add_Hinge_Constraint(btRigidBody & rbA, btRigidBody & rbB, const btVector3& pivotInA, const btVector3& pivotInB, btVector3& axisInA, btVector3& axisInB);
	btHingeConstraint*			Add_EnginedHinge_Constraint(btRigidBody & rbA, btRigidBody & rbB, const btVector3& pivotInA, const btVector3& pivotInB, btVector3& axisInA, btVector3& axisInB);
	btFixedConstraint*			Add_Fixed_Constraint(btRigidBody& rbA, btRigidBody& rbB, const btTransform& frameA, const btTransform& frameB);

};

class DebugDrawer : public btIDebugDraw
{
public:
	
	DebugDrawer();
	
public:

	DebugDrawModes	mode;
	_Line			line;
	_Primitive		point;

public:

	void drawLine(const btVector3& from, const btVector3& to, const btVector3& color);
	void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB, btScalar distance, int lifeTime, const btVector3& color);
	void reportErrorWarning(const char* warningString);
	void draw3dText(const btVector3& location, const char* textString);
	void setDebugMode(int debugMode);
	int	 getDebugMode() const;

};

#endif