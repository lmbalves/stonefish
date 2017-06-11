//
//  FixedJoint.h
//  Stonefish
//
//  Created by Patryk Cieslak on 2/4/13.
//  Copyright (c) 2013 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_FixedJoint__
#define __Stonefish_FixedJoint__

#include "Joint.h"

class FixedJoint : public Joint
{
public:
    FixedJoint(std::string uniqueName, SolidEntity* solidA, SolidEntity* solidB);
	FixedJoint(std::string uniqueName, btMultiBody* mb, btRigidBody* rb);
    
    btVector3 Render();
    JointType getType();
    
    //Not applicable
    void ApplyDamping(){}
    bool SolvePositionIC(btScalar linearTolerance, btScalar angularTolerance){ return true; }
};

#endif
