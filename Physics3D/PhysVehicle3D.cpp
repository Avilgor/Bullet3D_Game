#include "PhysVehicle3D.h"
#include "Primitive.h"
#include "ModulePhysics3D.h"
#include "Bullet/include/btBulletDynamicsCommon.h"

// ----------------------------------------------------------------------------
VehicleInfo::~VehicleInfo()
{
	/*if(wheels != NULL)
		delete wheels;*/
}

// ----------------------------------------------------------------------------
PhysVehicle3D::PhysVehicle3D(btRigidBody* body, btRaycastVehicle* vehicle, const VehicleInfo& info) : PhysBody3D(body), vehicle(vehicle), info(info)
{
	//Body = body;
}

// ----------------------------------------------------------------------------
PhysVehicle3D::~PhysVehicle3D()
{
	delete vehicle;
}

// ----------------------------------------------------------------------------
void PhysVehicle3D::Render()
{
	Cylinder wheel;

	wheel.color = Red;

	for(int i = 0; i < vehicle->getNumWheels(); ++i)
	{
		wheel.radius = info.wheels[0].radius;
		wheel.height = info.wheels[0].width;

		vehicle->updateWheelTransform(i);
		vehicle->getWheelInfo(i).m_worldTransform.getOpenGLMatrix(&wheel.transform);

		wheel.Render();
	}

	Cube chassis(info.chassis_size.x, info.chassis_size.y, info.chassis_size.z);
	vehicle->getChassisWorldTransform().getOpenGLMatrix(&chassis.transform);
	btQuaternion q = vehicle->getChassisWorldTransform().getRotation();
	btVector3 offset(info.chassis_offset.x, info.chassis_offset.y, info.chassis_offset.z);
	offset = offset.rotate(q.getAxis(), q.getAngle());

	Cube FrontChassis(info.front_chassis_size.x, info.front_chassis_size.y, info.front_chassis_size.z);
	vehicle->getChassisWorldTransform().getOpenGLMatrix(&FrontChassis.transform);
	q = vehicle->getChassisWorldTransform().getRotation();
	btVector3 offsetFront(info.front_chassis_offset.x, info.front_chassis_offset.y, info.front_chassis_offset.z);
	offsetFront = offsetFront.rotate(q.getAxis(), q.getAngle());

	Cube RearChassis(info.rear_chassis_size.x, info.rear_chassis_size.y, info.rear_chassis_size.z);
	RearChassis.color = Blue;
	vehicle->getChassisWorldTransform().getOpenGLMatrix(&RearChassis.transform);
	q = vehicle->getChassisWorldTransform().getRotation();
	btVector3 offsetRear(info.rear_chassis_offset.x, info.rear_chassis_offset.y, info.rear_chassis_offset.z);
	offsetRear = offsetRear.rotate(q.getAxis(), q.getAngle());

	Sphere cabine(info.cabine_radius);
	cabine.color = Red;
	vehicle->getChassisWorldTransform().getOpenGLMatrix(&cabine.transform);
	q = vehicle->getChassisWorldTransform().getRotation();
	btVector3 offsetCabine(info.cabine_offset.x, info.cabine_offset.y, info.cabine_offset.z);
	offsetCabine = offsetCabine.rotate(q.getAxis(), q.getAngle());

	Cube jointPrimitive(0.1f,0.1f,0.1f);
	jointPrimitive.color = White;
	vehicle->getChassisWorldTransform().getOpenGLMatrix(&jointPrimitive.transform);
	q = vehicle->getChassisWorldTransform().getRotation();
	btVector3 offsetJoint(info.jointOffset.x, info.jointOffset.y, info.jointOffset.z);
	offsetJoint = offsetJoint.rotate(q.getAxis(), q.getAngle());

	chassis.transform.M[12] += offset.getX();
	chassis.transform.M[13] += offset.getY();
	chassis.transform.M[14] += offset.getZ();

	FrontChassis.transform.M[12] += offsetFront.getX();
	FrontChassis.transform.M[13] += offsetFront.getY();
	FrontChassis.transform.M[14] += offsetFront.getZ();

	RearChassis.transform.M[12] += offsetRear.getX();
	RearChassis.transform.M[13] += offsetRear.getY();
	RearChassis.transform.M[14] += offsetRear.getZ();
	
	cabine.transform.M[12] += offsetCabine.getX();
	cabine.transform.M[13] += offsetCabine.getY();
	cabine.transform.M[14] += offsetCabine.getZ();

	jointPrimitive.transform.M[12] += offsetJoint.getX();
	jointPrimitive.transform.M[13] += offsetJoint.getY();
	jointPrimitive.transform.M[14] += offsetJoint.getZ();


	chassis.Render();
	FrontChassis.Render();
	RearChassis.Render();
	cabine.Render();
	jointPrimitive.Render();
}

// ----------------------------------------------------------------------------
void PhysVehicle3D::ApplyEngineForce(float force)
{
	for(int i = 0; i < vehicle->getNumWheels(); ++i)
	{
		if(info.wheels[i].drive == true)
		{
			vehicle->applyEngineForce(force, i);
		}
	}
}

// ----------------------------------------------------------------------------
void PhysVehicle3D::Brake(float force)
{
	for(int i = 0; i < vehicle->getNumWheels(); ++i)
	{
		if(info.wheels[i].brake == true)
		{
			vehicle->setBrake(force, i);
		}
	}
}

// ----------------------------------------------------------------------------
void PhysVehicle3D::Turn(float degrees)
{
	for(int i = 0; i < vehicle->getNumWheels(); ++i)
	{
		if(info.wheels[i].steering == true)
		{
			vehicle->setSteeringValue(degrees, i);
		}
	}
}

// ----------------------------------------------------------------------------
float PhysVehicle3D::GetKmh() const
{
	return vehicle->getCurrentSpeedKmHour();
}