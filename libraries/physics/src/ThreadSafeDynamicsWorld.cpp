/*
 * Bullet Continuous Collision Detection and Physics Library
 * Copyright (c) 2003-2009 Erwin Coumans  http://bulletphysics.org
 *
 * This software is provided 'as-is', without any express or implied warranty.
 * In no event will the authors be held liable for any damages arising from the use of this software.
 * Permission is granted to anyone to use this software for any purpose, 
 * including commercial applications, and to alter it and redistribute it freely, 
 * subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 * Copied and modified from btDiscreteDynamicsWorld.cpp by AndrewMeadows on 2014.11.12.
 * */

#include <LinearMath/btQuickprof.h>

#include "ObjectMotionState.h"
#include "ThreadSafeDynamicsWorld.h"

ThreadSafeDynamicsWorld::ThreadSafeDynamicsWorld(
        btDispatcher* dispatcher,
        btBroadphaseInterface* pairCache,
        btConstraintSolver* constraintSolver,
        btCollisionConfiguration* collisionConfiguration)
    :   btDiscreteDynamicsWorld(dispatcher, pairCache, constraintSolver, collisionConfiguration) {
}

int ThreadSafeDynamicsWorld::stepSimulation( btScalar timeStep, int maxSubSteps, btScalar fixedTimeStep) {
    BT_PROFILE("stepSimulation");
    int subSteps = 0;
    if (maxSubSteps) {
        //fixed timestep with interpolation
        m_fixedTimeStep = fixedTimeStep;
        m_localTime += timeStep;
        if (m_localTime >= fixedTimeStep)
        {
            subSteps = int( m_localTime / fixedTimeStep);
            m_localTime -= subSteps * fixedTimeStep;
        }
    } else {
        //variable timestep
        fixedTimeStep = timeStep;
        m_localTime = m_latencyMotionStateInterpolation ? 0 : timeStep;
        m_fixedTimeStep = 0;
        if (btFuzzyZero(timeStep))
        {
            subSteps = 0;
            maxSubSteps = 0;
        } else
        {
            subSteps = 1;
            maxSubSteps = 1;
        }
    }

    /*//process some debugging flags
    if (getDebugDrawer()) {
        btIDebugDraw* debugDrawer = getDebugDrawer();
        gDisableDeactivation = (debugDrawer->getDebugMode() & btIDebugDraw::DBG_NoDeactivation) != 0;
    }*/
    if (subSteps) {
        //clamp the number of substeps, to prevent simulation grinding spiralling down to a halt
        int clampedSimulationSteps = (subSteps > maxSubSteps)? maxSubSteps : subSteps;

        saveKinematicState(fixedTimeStep*clampedSimulationSteps);

        applyGravity();

        for (int i=0;i<clampedSimulationSteps;i++) {
            internalSingleStepSimulation(fixedTimeStep);
        }
    }

    // NOTE: We do NOT call synchronizeMotionState() after each substep (to avoid multiple locks on the
    // object data outside of the physics engine).  A consequence of this is that the transforms of the
    // external objects only ever update at the end of the full step.

    // NOTE: We do NOT call synchronizeMotionStates() here.  Instead it is called by an external class
    // that knows how to lock threads correctly.

    clearForces();

    return subSteps;
}

void ThreadSafeDynamicsWorld::synchronizeMotionStates() {
    _changedMotionStates.clear();
    BT_PROFILE("synchronizeMotionStates");
    if (m_synchronizeAllMotionStates) {
        //iterate  over all collision objects
        for (int i=0;i<m_collisionObjects.size();i++) {
            btCollisionObject* colObj = m_collisionObjects[i];
            btRigidBody* body = btRigidBody::upcast(colObj);
            if (body) {
                if (body->getMotionState()) {
                    synchronizeSingleMotionState(body);
                    _changedMotionStates.push_back(static_cast<ObjectMotionState*>(body->getMotionState()));
                }
            }
        }
    } else  {       
        //iterate over all active rigid bodies
        for (int i=0;i<m_nonStaticRigidBodies.size();i++) {
            btRigidBody* body = m_nonStaticRigidBodies[i];
            if (body->isActive()) {
                if (body->getMotionState()) {
                    synchronizeSingleMotionState(body);
                    _changedMotionStates.push_back(static_cast<ObjectMotionState*>(body->getMotionState()));
                }
            }
        }
    }   
}       

