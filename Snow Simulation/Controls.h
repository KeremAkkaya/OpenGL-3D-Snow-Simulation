#pragma once

// Include GLFW
#include <GLFW\glfw3.h>

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;

// Initial position : on +Z
glm::vec3 position = glm::vec3(50, 45, -50);

glm::vec3 controlledCubePostion = glm::vec3(0, 10, 0);
glm::vec3 controlledCubeScale = glm::vec3(1, 1, 1);

bool resetAccumulation = false;
bool isSnowOn = true;
bool isMeltOn = true;
bool isDebugLines = false;
bool isCursorLocked = true;
bool secondMarker = true;

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}
glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}
glm::vec3 getPositionVec() {
	return position;
}

glm::vec3 getControlledCubePostionVec() {
	return controlledCubePostion;
}

// Initial horizontal angle : toward -Z
float horizontalAngle = 0.5f;
// Initial vertical angle : none
float verticalAngle = -0.6f;
// Initial Field of View
float initialFoV = 70.0f;

float speed = 30.0f; // 3 units / second
float mouseSpeed = 0.0005f;
double windSpeed = 1.3; // Model rotation speed

#include <glm\glm.hpp>

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0)
		windSpeed = clamp(windSpeed + 0.3, 0.1, 10.0);
	else
		windSpeed = clamp(windSpeed - 0.3, 0.1, 10.0);
}

void computeMatricesFromInputs(GLFWwindow* window) {

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	if (isCursorLocked)
	{
		// Get mouse position
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);

		// Reset mouse position for next frame
		glfwSetCursorPos(window, 1024 / 2, 768 / 2);

		// Compute new orientation
		horizontalAngle += mouseSpeed * float(1024 / 2 - xpos);
		verticalAngle += mouseSpeed * float(768 / 2 - ypos);

	}
		//reset to 0
		if (horizontalAngle > two_pi<float>() || horizontalAngle < -two_pi<float>())
		{
			horizontalAngle = 0;
		}
		if (verticalAngle > two_pi<float>() || verticalAngle < -two_pi<float>())
		{
			verticalAngle = 0;
		}

		// Direction : Spherical coordinates to Cartesian coordinates conversion
		glm::vec3 direction(
			cos(verticalAngle) * sin(horizontalAngle),
			sin(verticalAngle),
			cos(verticalAngle) * cos(horizontalAngle)
		);

		// Right vector
		glm::vec3 right = glm::vec3(
			sin(horizontalAngle - 3.14f / 2.0f),
			0,
			cos(horizontalAngle - 3.14f / 2.0f)
		);

		// Up vector
		glm::vec3 up = glm::cross(right, direction);
	
	// Move forward
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
		position += direction * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
		position -= direction * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
		position -= right * deltaTime * speed;
	}	

	// Down
	if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS) {
		position.y += deltaTime * speed;
	}
	// Up
	if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS) {
		position.y -= deltaTime * speed;
		if (position.y < 0)
			position.y = 0;
	}

	//TOGGLES
	if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
		isSnowOn = true;
	}
	if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
		isSnowOn = false;
	}
	if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
		isMeltOn = true;
	}
	if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
		isMeltOn = false;
	}
	if (glfwGetKey(window, GLFW_KEY_9) == GLFW_PRESS) {
		isDebugLines = true;
	}
	if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS) {
		isDebugLines = false;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
		isCursorLocked = !isCursorLocked;
	}

	//CONTROLLED CUBE
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		controlledCubePostion.z += deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		controlledCubePostion.z -= deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		controlledCubePostion.x += deltaTime * speed;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		controlledCubePostion.x -= deltaTime * speed;
	}
	// Down
	if (glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS) {
		controlledCubePostion.y -= deltaTime * speed;
	}
	// Up
	if (glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS) {
		controlledCubePostion.y += deltaTime * speed;
	}
	// ScaleDown
	if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) {
		controlledCubeScale.z = clamp(controlledCubeScale.z - 0.1, 0.1, 10.0);
	}
	// ScaleUp
	if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) {
		controlledCubeScale.z = clamp(controlledCubeScale.z + 0.1, 0.1, 10.0);
	}
	if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS) {
		secondMarker = true;
	}
	if (glfwGetKey(window, GLFW_KEY_KP_3) == GLFW_PRESS) {
		secondMarker = false;
	}

	//RESET
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		resetAccumulation = true;
	}

	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

						   // Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, 0.1f, 10000.0f);
	// Camera matrix
	ViewMatrix = glm::lookAt(
		position,           // Camera is here
		position + direction, // and looks here : at the same position, plus "direction"
		up                  // Head is up (set to 0,-1,0 to look upside-down)
	);



	//std::cout << position.x << " " << position.y << " " << position.z << "		" <<  horizontalAngle << "		" << verticalAngle << std::endl;

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}
