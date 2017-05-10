#pragma once
#include <vector>
#include <algorithm>
#include <GL/glew.h>
#include <gl\glut.h>
#include <gl\freeglut.h>
#include <GL\freeglut.h>
#include <GL/freeglut_ext.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <thread>

using namespace std;
using namespace glm;

struct Point {
	Point* left = NULL;
	Point* right = NULL;
	Point* down = NULL;
	Point* up = NULL;

	vec3 Location;
};

class Accumulation
{
public:
	int heightDivisions;
	int widthDivisions;
	float initHeight;

	GLuint vbo[3];
	GLuint surface_ibo;
	vec3 vertices[201][201];

	GLuint vaID;
	GLuint vbID;
	GLuint ColourID;
	int segmentLength;

	Accumulation(GLuint _programID);
	~Accumulation();
	void Update();
	void Draw();
	void AddPoint(vec3 _newPoint);
	void RemovePoint(vec3 _newPoint, vec3 _scale);
	void Melt();

	void ThreadedAddPoint(vec3 _newPoint);

	vector<vec3> gridPoints;
	vector<vec3> meshPoints;
	vector<int> meshIndices;
	vector<vec3> gridMeshPoints;
	Point* grid[100][100];


	int RoundUp(int toRound);
	int RoundDown(int toRound);
	void ResetAccumulation();

	vector<std::thread> threads;

	friend bool operator== (vec3 &v1, vec3 &v2) {
		return (v1.x == v2.x && v1.z == v2.z);
	};

private:
	static Accumulation* _instance;

	/* A utility function to calculate area of triangle formed by (x1, y1),
	(x2, y2) and (x3, y3) */
	float area(int x1, int y1, int x2, int y2, int x3, int y3)
	{
		return abs((x1*(y2 - y3) + x2*(y3 - y1) + x3*(y1 - y2)) / 2.0);
	}

	/* A function to check whether point P(x, y) lies inside the triangle formed
	by A(x1, y1), B(x2, y2) and C(x3, y3) */
	bool isInside(int x1, int y1, int x2, int y2, int x3, int y3, int x, int y)
	{
		/* Calculate area of triangle ABC */
		float A = area(x1, y1, x2, y2, x3, y3);

		/* Calculate area of triangle PBC */
		float A1 = area(x, y, x2, y2, x3, y3);

		/* Calculate area of triangle PAC */
		float A2 = area(x1, y1, x, y, x3, y3);

		/* Calculate area of triangle PAB */
		float A3 = area(x1, y1, x2, y2, x, y);

		/* Check if sum of A1, A2 and A3 is same as A */
		return (A == A1 + A2 + A3);
	}


};

