#pragma once
#include <GL/glew.h>
#include <gl\glut.h>
#include <gl\freeglut.h>
#include <GL\freeglut.h>
#include <GL/freeglut_ext.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
using namespace glm;
#include <iostream>
#include <fstream>
#include <glm/gtc/type_ptr.hpp> // glm::value_ptr

class Deformation
{
public:
	Deformation(GLuint _programID);
	~Deformation();

	GLuint Draw(vec3 position);
	vec3 scale;

private:
	unsigned char * textureDataBuffer;
	GLuint TextureID;
	GLuint programID;
	GLuint vertexbuffer;
	GLuint uvbuffer;
	GLuint VertexArrayID;
	vec3 position;
	GLuint modelMatrixID;
	GLuint ModelMatrixID;

	GLuint ColourID;

};

