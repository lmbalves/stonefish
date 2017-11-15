//
//  Compound.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 19/09/17.
//  Copyright (c) 2017 Patryk Cieslak. All rights reserved.
//

#include "Compound.h"
#include "MathsUtil.hpp"
#include "SimulationApp.h"

Compound::Compound(std::string uniqueName, SolidEntity* firstExternalPart, const btTransform& position) : SolidEntity(uniqueName, firstExternalPart->getMaterial(), firstExternalPart->getLook())
{
    volume = 0;
	mass = 0;
	Ipri = btVector3(0,0,0);
	mesh = NULL;
	AddExternalPart(firstExternalPart, position);
}

Compound::~Compound()
{
    for(unsigned int i=0; i<parts.size(); ++i)
        delete parts[i].solid;
    
    parts.clear();
}

SolidType Compound::getSolidType()
{
    return SOLID_COMPOUND;
}

std::vector<Vertex>* Compound::getMeshVertices()
{
    std::vector<Vertex>* pVert = new std::vector<Vertex>(0);
    
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        if(parts[i].isExternal)
        {
            std::vector<Vertex>* pPartVert = parts[i].solid->getMeshVertices();
            
            for(unsigned int h=0; h < pPartVert->size(); ++h)
            {
                glm::mat4 trans = glMatrixFromBtTransform(parts[i].position);
                glm::vec4 vTrans = trans * glm::vec4((*pPartVert)[h].pos, 1.f);
                Vertex v;
                v.pos = glm::vec3(vTrans);
                pVert->push_back(v);
            }
            
            delete pPartVert;
        }
    }
    
    return pVert;
}

void Compound::AddInternalPart(SolidEntity* solid, const btTransform& position)
{
    if(solid != NULL)
    {
        Part part;
        part.solid = solid;
        part.position = position;
        part.isExternal = false;
        parts.push_back(part);
		RecalculatePhysicalProperties();
    }
}

void Compound::AddExternalPart(SolidEntity* solid, const btTransform& position)
{
    if(solid != NULL)
    {
        Part part;
        part.solid = solid;
        part.position = position;
        part.isExternal = true;
        parts.push_back(part);
		RecalculatePhysicalProperties();
    }
}

void Compound::RecalculatePhysicalProperties()
{
	//Calculate rigid body properties
    /*
      1. Calculate compound mass and compound COG (sum of location - local * m / M)
      2. Calculate inertia of part in global frame and compound COG (rotate and translate inertia tensor) 3x3
         and calculate compound inertia 3x3 (sum of parts inertia)
      3. Calculate primary moments of inertia
      4. Find primary axes of inertia
      5. Rotate frame to match primary axes and move to COG
    */
     
    //1. Calculate compound mass and COG
    btVector3 compoundCOG(0,0,0);
    btVector3 compoundCOB(0,0,0);
    btScalar compoundMass = 0;
	btScalar compoundVolume = 0;
    localTransform = btTransform::getIdentity();
        
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        compoundMass += parts[i].solid->getMass();
        //compoundCOG += (bodyParts[i].position.getOrigin() + bodyParts[i].solid->getLocalTransform().getOrigin())*bodyParts[i].solid->getMass();
        compoundCOG += (parts[i].position*parts[i].solid->getGeomToCOGTransform().getOrigin())*parts[i].solid->getMass();
        
        if(parts[i].solid->isBuoyant())
        {
            compoundVolume += parts[i].solid->getVolume();
            compoundCOB += (parts[i].position*parts[i].solid->getCOBInGeomFrame())*parts[i].solid->getVolume();
        }
    }
    
    compoundCOG /= compoundMass;
    localTransform.setOrigin(compoundCOG);
    
    if(compoundVolume > btScalar(0))
        CoB = compoundCOB / compoundVolume;
        
    //2. Calculate compound inertia matrix
    btMatrix3x3 I = btMatrix3x3(0,0,0,0,0,0,0,0,0);
        
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        //Calculate inertia matrix 3x3 of solid in the global frame and COG
        btVector3 solidPriInertia = parts[i].solid->getMomentsOfInertia();
        btMatrix3x3 solidInertia = btMatrix3x3(solidPriInertia.x(), 0, 0, 0, solidPriInertia.y(), 0, 0, 0, solidPriInertia.z());
            
        //Rotate inertia tensor from local to global
        btMatrix3x3 rotation = parts[i].position.getBasis()*parts[i].solid->getGeomToCOGTransform().getBasis();
        solidInertia = rotation * solidInertia * rotation.transpose();
            
        //Translate inertia tensor from local COG to global COG
        btVector3 translation = parts[i].position.getOrigin()+parts[i].solid->getGeomToCOGTransform().getOrigin()-compoundCOG;
        solidInertia = solidInertia +  btMatrix3x3(translation.y()*translation.y()+translation.z()*translation.z(), -translation.x()*translation.y(), -translation.x()*translation.z(),
                                                    -translation.y()*translation.x(), translation.x()*translation.x()+translation.z()*translation.z(), -translation.y()*translation.z(),
                                                    -translation.z()*translation.x(), -translation.z()*translation.y(), translation.x()*translation.x()+translation.y()*translation.y()).scaled(btVector3(parts[i].solid->getMass(), parts[i].solid->getMass(), parts[i].solid->getMass()));
            
        //Accumulate inertia tensor
        I += solidInertia;
    }
	
	//3. Find compound moments of inertia
	btVector3 compoundPriInertia(I.getRow(0).getX(), I.getRow(1).getY(), I.getRow(2).getZ());
	
	//Check if inertia matrix is not diagonal
	if(!(btFuzzyZero(I.getRow(0).getY()) && btFuzzyZero(I.getRow(0).getZ())
	     && btFuzzyZero(I.getRow(1).getX()) && btFuzzyZero(I.getRow(1).getZ())
	     && btFuzzyZero(I.getRow(2).getX()) && btFuzzyZero(I.getRow(2).getY())))
	{
		//3.1. Calculate principal moments of inertia
		btScalar T = I[0][0] + I[1][1] + I[2][2]; //Ixx + Iyy + Izz
		btScalar II = I[0][0]*I[1][1] + I[0][0]*I[2][2] + I[1][1]*I[2][2] - I[0][1]*I[0][1] - I[0][2]*I[0][2] - I[1][2]*I[1][2]; //Ixx Iyy + Ixx Izz + Iyy Izz - Ixy^2 - Ixz^2 - Iyz^2
		btScalar U = btSqrt(T*T-btScalar(3.)*II)/btScalar(3.);
		btScalar theta = btAcos((-btScalar(2.)*T*T*T + btScalar(9.)*T*II - btScalar(27.)*I.determinant())/(btScalar(54.)*U*U*U));
		btScalar A = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.));
		btScalar B = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.) - btScalar(2.)*M_PI/btScalar(3.));
		btScalar C = T/btScalar(3.) - btScalar(2.)*U*btCos(theta/btScalar(3.) + btScalar(2.)*M_PI/btScalar(3.));
		compoundPriInertia = btVector3(A, B, C);
    
		//3.2. Calculate principal axes of inertia
		btMatrix3x3 L;
		btVector3 axis1,axis2,axis3;
		axis1 = findInertiaAxis(I, A);
		axis2 = findInertiaAxis(I, B);
		axis3 = axis1.cross(axis2);
		axis2 = axis3.cross(axis1);
    
		//3.3. Rotate body so that principal axes are parallel to (x,y,z) system
		btMatrix3x3 rotMat(axis1[0],axis2[0],axis3[0], axis1[1],axis2[1],axis3[1], axis1[2],axis2[2],axis3[2]);
		localTransform.setBasis(rotMat);
    }
	
	mass = compoundMass;
	volume = compoundVolume;
	Ipri = compoundPriInertia;
    
    ComputeEquivEllipsoid();
}

btCollisionShape* Compound::BuildCollisionShape()
{
	//Build collision shape from external parts
    btCompoundShape* colShape = new btCompoundShape();
    for(unsigned int i=0; i<parts.size(); ++i)
    {
        if(parts[i].isExternal)
        {
            btTransform childTrans = parts[i].position;
            colShape->addChildShape(childTrans, parts[i].solid->BuildCollisionShape());
        }
    }
	
	colShape->setMargin(UnitSystem::Length(UnitSystems::MKS, UnitSystem::GetInternalUnitSystem(), btScalar(0.001)));
	return colShape;
}

void Compound::ComputeFluidForces(HydrodynamicsSettings settings, const Liquid* liquid)
{
    btTransform T = getTransform() * localTransform.inverse();
    btVector3 v = getLinearVelocity();
    btVector3 omega = getAngularVelocity();
	btVector3 a = getLinearAcceleration();
    btVector3 epsilon = getAngularAcceleration();
	
	Fb.setZero();
	Tb.setZero();
	Fd.setZero();
	Td.setZero();
	Fa.setZero();
	Ta.setZero();
	
	btVector3 Fbp(0,0,0);
	btVector3 Tbp(0,0,0);
	btVector3 Fdp(0,0,0);
	btVector3 Tdp(0,0,0);
	btVector3 Fap(0,0,0);
	btVector3 Tap(0,0,0);
	
    btVector3 aabbMin, aabbMax;
    GetAABB(aabbMin, aabbMax);
    
    if(liquid->GetDepth(aabbMin) > btScalar(0) && liquid->GetDepth(aabbMax) > btScalar(0))
    {
        Fb = -volume*liquid->getFluid()->density * SimulationApp::getApp()->getSimulationManager()->getGravity();
        Tb = (T*CoB - getTransform().getOrigin()).cross(Fb);
        settings.reallisticBuoyancy = false; //disable buoyancy calculation
    }
    
	for(unsigned int i=0; i<parts.size(); ++i)
	{
		if(parts[i].isExternal)
		{
			parts[i].solid->ComputeFluidForces(settings, liquid, getTransform(), T * parts[i].position, v, omega, a, epsilon, Fbp, Tbp, Fdp, Tdp, Fap, Tap);
			Fb += Fbp;
			Tb += Tbp;
			Fd += Fdp;
			Td += Tdp;
			Fa += Fap;
			Ta += Tap;
		}
		else
		{
			HydrodynamicsSettings iSettings = settings;
			iSettings.addedMassForces = false;
			iSettings.dampingForces = false;
			parts[i].solid->ComputeFluidForces(iSettings, liquid, getTransform(), T * parts[i].position, v, omega, a, epsilon, Fbp, Tbp, Fdp, Tdp, Fap, Tap);
			Fb += Fbp;
			Tb += Tbp;
		}
	}
    
    //Correct drag based on ellipsoid approximation of shape
    btTransform eTrans = getTransform() * localTransform.inverse() * ellipsoidTransform;
    btVector3 eFd = eTrans.getBasis().inverse() * Fd;
    //btVector3 eTd = eTrans.getBasis().inverse() * Td;
    btVector3 Cd(btScalar(1)/ellipsoidR.x(), btScalar(1)/ellipsoidR.y(), btScalar(1)/ellipsoidR.z());
    btScalar maxCd = btMax(btMax(Cd.x(), Cd.y()), Cd.z());
    Cd /= maxCd;
    eFd = btVector3(Cd.x()*eFd.x(), Cd.y()*eFd.y(), Cd.z()*eFd.z());
    Fd = eTrans.getBasis() * eFd;
}

void Compound::BuildGraphicalObject()
{
	for(unsigned int i=0; i<parts.size(); ++i)
		parts[i].solid->BuildGraphicalObject();
}

std::vector<Renderable> Compound::Render()
{
	std::vector<Renderable> items(0);
	
	if(isRenderable())
	{
		btTransform cgCompoundTrans = getTransform();
		btTransform oCompoundTrans =  getTransform() * localTransform.inverse();
	
        Renderable item;
        item.type = RenderableType::SOLID_CS;
        item.model = glMatrixFromBtTransform(cgCompoundTrans);
        items.push_back(item);
        
        btVector3 cobWorld = oCompoundTrans * CoB;
        item.type = RenderableType::HYDRO_CS;
        item.model = glMatrixFromBtTransform(btTransform(btQuaternion::getIdentity(), cobWorld));
        item.points.push_back(glm::vec3(volume, volume, volume));
        items.push_back(item);
    
		for(unsigned int i=0; i<parts.size(); ++i)
		{
			btTransform oTrans = oCompoundTrans * parts[i].position;
			btTransform cgTrans = cgCompoundTrans; //Multiple rendering of the same coordinate system!!!!!
			//btTransform cgTrans = oTrans * parts[i].solid->getGeomToCOGTransform();
            
			Renderable item;
            item.type = RenderableType::SOLID;
			item.objectId = parts[i].solid->getObject();
			item.lookId = parts[i].solid->getLook();
			item.model = glMatrixFromBtTransform(oTrans);
			items.push_back(item);
            
            item.type = RenderableType::HYDRO;
            item.model = glMatrixFromBtTransform(oCompoundTrans * ellipsoidTransform);
            item.points.push_back(glm::vec3((GLfloat)ellipsoidR[0], (GLfloat)ellipsoidR[1], (GLfloat)ellipsoidR[2]));
			items.push_back(item);
		}
	}
		
	return items;
}