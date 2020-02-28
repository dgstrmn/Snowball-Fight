#pragma once

#include <GL\glew.h>

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>

#include <GLFW\glfw3.h>


#include <stdio.h>

class Camera
{
public:
	Camera();
	Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed);

	void keyControl(bool* keys, GLfloat deltaTime, GLfloat curHeight, float border);
	void mouseControl(bool* keys, GLfloat xChange, GLfloat yChange, bool &left_pressed);

	glm::vec3 getCameraPosition();
	GLfloat getCameraYaw();
	GLfloat getCameraPitch();
	glm::vec3 getCameraFront();

	glm::mat4 calculateViewMatrix();

	~Camera();

private:
	glm::vec3 position;
	glm::vec3 lastPos;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;

	GLfloat yaw;
	GLfloat pitch;

	GLfloat moveSpeed;
	GLfloat turnSpeed;
	GLfloat jumpHeight;
	GLfloat acc_grav;

	bool onAir;

	void update();
};

