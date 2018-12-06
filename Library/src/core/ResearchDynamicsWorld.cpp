//
//  ResearchDynamicsWorld.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 11/05/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#include "core/ResearchDynamicsWorld.h"

#include "core/ResearchConstraintSolver.h"

namespace sf
{

ResearchDynamicsWorld::ResearchDynamicsWorld(btDispatcher* dispatcher, btBroadphaseInterface* pairCache, ResearchConstraintSolver* constraintSolver, btCollisionConfiguration* collisionConfiguration) : btMultiBodyDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration)
{
}
    
}