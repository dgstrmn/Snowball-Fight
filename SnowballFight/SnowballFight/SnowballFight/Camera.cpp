#include "Camera.h"

Camera::Camera() {}

Camera::Camera(glm::vec3 startPosition, glm::vec3 startUp, GLfloat startYaw, GLfloat startPitch, GLfloat startMoveSpeed, GLfloat startTurnSpeed)
{
	position = startPosition;
	worldUp = startUp;
	yaw = startYaw;
	pitch = startPitch;
	front = glm::vec3(0.0f, 0.0f, -1.0f);

	moveSpeed = startMoveSpeed;
	turnSpeed = startTurnSpeed;
	acc_grav = 3.0f;
	onAir = false;

	update();
}

void Camera::keyControl(bool* keys, GLfloat deltaTime, GLfloat curHeight, float border)
{
	GLfloat velocity = moveSpeed * deltaTime;

	if (keys[GLFW_KEY_SPACE] && !onAir)
	{
		onAir = true;
		jumpHeight = 1.0f;
	}
	if (onAir)
	{
		position.y += jumpHeight;
		jumpHeight -= acc_grav * deltaTime;
	}
	if(position.y <= curHeight || !onAir)
	{
		onAir = false;
		position.y = curHeight;
	}

	lastPos = position;
	if (keys[GLFW_KEY_W])
	{
		position.x+= (front * velocity).x;
		position.z += (front * velocity).z;
	}
	if (std::abs(position.x) > border || std::abs(position.z) > border)
	{
		position = lastPos;
	}

	lastPos = position;
	if (keys[GLFW_KEY_S])
	{
		position.x -= (front * velocity).x;
		position.z -= (front * velocity).z;
	}
	if (std::abs(position.x) > border || std::abs(position.z) > border)
	{
		position = lastPos;
	}

	lastPos = position;
	if (keys[GLFW_KEY_A])
	{
		position -= right * velocity;
	}
	if (std::abs(position.x) > border || std::abs(position.z) > border)
	{
		position = lastPos;
	}

	lastPos = position;
	if (keys[GLFW_KEY_D])
	{
		position += right * velocity;
	}
	if (std::abs(position.x) > border || std::abs(position.z) > border)
	{
		position = lastPos;
	}
	
}

void Camera::mouseControl(bool *keys, GLfloat xChange, GLfloat yChange, bool &left_pressed)
{
	xChange *= turnSpeed;
	yChange *= turnSpeed;

	yaw += xChange;
	pitch += yChange;

	if (pitch > 89.9f)
	{
		pitch = 89.9f;
	}

	if (pitch < -89.9f)
	{
		pitch = -89.9f;
	}

	if (keys[GLFW_MOUSE_BUTTON_LEFT])
	{
		left_pressed = true;
	}
	else
	{
		left_pressed = false;
	}

	update();
}

glm::mat4 Camera::calculateViewMatrix()
{
	return glm::lookAt(position, position + front, up);
}

glm::vec3 Camera::getCameraPosition()
{
	return position;
}

GLfloat Camera::getCameraYaw()
{
	return yaw;
}

GLfloat Camera::getCameraPitch()
{
	return pitch;
}

glm::vec3 Camera::getCameraFront()
{
	return front;
}

void Camera::update()
{
	front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	front.y = sin(glm::radians(pitch));
	front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	front = glm::normalize(front);

	right = glm::normalize(glm::cross(front, worldUp));
	up = glm::normalize(glm::cross(right, front));
}


Camera::~Camera()
{
}
