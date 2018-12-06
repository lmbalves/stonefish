//
//  Camera.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 4/7/17.
//  Copyright (c) 2017-2018 Patryk Cieslak. All rights reserved.
//

#include "sensors/vision/Camera.h"

#include "core/SimulationApp.h"
#include "core/Console.h"
#include "entities/SolidEntity.h"

namespace sf
{

Camera::Camera(std::string uniqueName, uint32_t resX, uint32_t resY, Scalar horizFOVDeg, Scalar frequency) : VisionSensor(uniqueName, frequency)
{
	if(!SimulationApp::getApp()->hasGraphics())
		cCritical("Not possible to use cameras in console simulation! Use graphical simulation if possible.");
	
	fovH = horizFOVDeg;
    resx = resX;
    resy = resY;
    pan = Scalar(0);
    tilt = Scalar(0);
    display = false;
}
    
Camera::~Camera()
{
}

Scalar Camera::getHorizontalFOV()
{
    return fovH;
}

void Camera::getResolution(uint32_t& x, uint32_t& y)
{
    x = resx;
    y = resy;
}

void Camera::setDisplayOnScreen(bool onScreen)
{
    display = onScreen;
}

bool Camera::getDisplayOnScreen()
{
    return display;
}

void Camera::setPan(Scalar value)
{
    pan = value;
    //SetupCamera();
}

void Camera::setTilt(Scalar value)
{
    tilt = value;
    //SetupCamera();
}

Scalar Camera::getPan()
{
    return pan;
}

Scalar Camera::getTilt()
{
    return tilt;
}

void Camera::UpdateTransform()
{
    Transform cameraTransform = getSensorFrame();
    Vector3 eyePosition = cameraTransform.getOrigin(); //O
    Vector3 direction = cameraTransform.getBasis().getColumn(2); //Z
    Vector3 cameraUp = -cameraTransform.getBasis().getColumn(1); //-Y
    
    //additional camera rotation
    /*glm::vec3 tiltAxis = glm::normalize(glm::cross(dir, up));
    glm::vec3 panAxis = glm::normalize(glm::cross(tiltAxis, dir));
    
    //rotate
	lookingDir = glm::rotate(lookingDir, tilt, tiltAxis);
	lookingDir = glm::rotate(lookingDir, pan, panAxis);
    lookingDir = glm::normalize(lookingDir);
    
    currentUp = glm::rotate(currentUp, tilt, tiltAxis);
    currentUp = glm::rotate(currentUp, pan, panAxis);
	currentUp = glm::normalize(currentUp);*/
    
    Matrix3 rotation;
    rotation.setEulerYPR(0,M_PI,0);
    Vector3 rotEyePosition = rotation * eyePosition;
    Vector3 rotDirection = rotation * direction;
    Vector3 rotCameraUp = rotation * cameraUp;
    SetupCamera(rotEyePosition, rotDirection, rotCameraUp);
}

std::vector<Renderable> Camera::Render()
{
    std::vector<Renderable> items(0);
    
    Renderable item;
    item.model = glMatrixFromTransform(getSensorFrame());
    item.type = RenderableType::SENSOR_LINES;
    
    //Create camera dummy
    GLfloat iconSize = 1.f;
    GLfloat x = iconSize*tanf(fovH/360.f*M_PI);
    GLfloat aspect = (GLfloat)resx/(GLfloat)resy;
    GLfloat y = x/aspect;
	
	item.points.push_back(glm::vec3(0,0,0));
	item.points.push_back(glm::vec3(x, -y, iconSize));
	item.points.push_back(glm::vec3(0,0,0));
	item.points.push_back(glm::vec3(x,  y, iconSize));
	item.points.push_back(glm::vec3(0,0,0));
	item.points.push_back(glm::vec3(-x, -y, iconSize));
	item.points.push_back(glm::vec3(0,0,0));
	item.points.push_back(glm::vec3(-x,  y, iconSize));
	
	item.points.push_back(glm::vec3(x, -y, iconSize));
	item.points.push_back(glm::vec3(x, y, iconSize));
	item.points.push_back(glm::vec3(x, y, iconSize));
	item.points.push_back(glm::vec3(-x, y, iconSize));
	item.points.push_back(glm::vec3(-x, y, iconSize));
	item.points.push_back(glm::vec3(-x, -y, iconSize));
	item.points.push_back(glm::vec3(-x, -y, iconSize));
	item.points.push_back(glm::vec3(x, -y, iconSize));
    
    item.points.push_back(glm::vec3(-0.5f*x, -y, iconSize));
	item.points.push_back(glm::vec3(0.f, -1.5f*y, iconSize));
	item.points.push_back(glm::vec3(0.f, -1.5f*y, iconSize));
	item.points.push_back(glm::vec3(0.5f*x, -y, iconSize));
    
    items.push_back(item);
    return items;
}

}