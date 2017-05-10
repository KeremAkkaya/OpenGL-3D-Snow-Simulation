#include <gl\glew.h>
#include <iostream>
#include "Accumulation.h"
using namespace glm;
using namespace std;

const int height = 201;
const int width = 201;
const float gain = 0.5f;
const float loss = -1.5f;

Accumulation::Accumulation(GLuint _programID)
{
	heightDivisions = 100;
	widthDivisions = 100;
	initHeight = -0.1f;
	segmentLength = height / heightDivisions;

	ColourID = glGetUniformLocation(_programID, "userColour");

	//https://en.wikibooks.org/wiki/OpenGL_Programming/Scientific_OpenGL_Tutorial_05
	glGenBuffers(3, vbo);
	for (int i = 0; i < 201; i++) {
		for (int j = 0; j < 201; j++) {
			vertices[i][j].x = i;
			vertices[i][j].y = initHeight;
			vertices[i][j].z = j;
		}
	}

	// Tell OpenGL to copy our array to the buffer objects
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

	GLushort indices[200 * 200 * 6];
	int i = 0;

	// Triangles
	for (int y = 0; y < 200; y++) {
		for (int x = 0; x < 200; x++) {
			indices[i++] = y * 200 + x;
			indices[i++] = y * 200 + x + 1;
			indices[i++] = (y + 1) * 200 + x + 1;

			indices[i++] = y * 200 + x;
			indices[i++] = (y + 1) * 200 + x + 1;
			indices[i++] = (y + 1) * 200 + x;
		}
	}

	glGenBuffers(1, &surface_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface_ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof indices, indices, GL_STATIC_DRAW);
}

void Accumulation::ThreadedAddPoint(vec3 _newPoint)
{
	int x = _newPoint.x;
	int z = _newPoint.z;

	int remainder = x % segmentLength;
	if (!remainder == 0)
		x = x + segmentLength - remainder;

	remainder = z % segmentLength;
	if (!remainder == 0)
		z = z + segmentLength - remainder;


	for (size_t i = 0; i < gridPoints.size(); i++)
	{
		if (gridPoints[i].x == x && gridPoints[i].z == z)
		{
			gridPoints[i].y += 1;
			break;
		}
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * gridPoints.size(), &gridPoints[0].x, GL_DYNAMIC_DRAW);
}

void Accumulation::AddPoint(vec3 _newPoint)
{
	int x = _newPoint.x;
	int z = _newPoint.z;
	int remainder = x % segmentLength;
	if (!remainder == 0)
		x = x + segmentLength - remainder;

	remainder = z % segmentLength;
	if (!remainder == 0)
		z = z + segmentLength - remainder;

	
	if (x < height - 1 && z < height - 1 && x > 1 && z > 1) {
		//vertices[x][z].y += 2.55f;
		//if (vertices[x + 1][z].y < vertices[x][z].y)
		//	vertices[x + 1][z].y = vertices[x][z].y;
		//if (vertices[x][z + 1].y < vertices[x][z].y)
		//	vertices[x][z + 1].y = vertices[x][z].y;
		//if (vertices[x + 1][z + 1].y < vertices[x][z].y)
		//	vertices[x + 1][z + 1].y = vertices[x][z].y;
		//if (vertices[x - 1][z].y < vertices[x][z].y)
		//	vertices[x - 1][z].y = vertices[x][z].y;
		//if (vertices[x][z - 1].y < vertices[x][z].y)
		//	vertices[x][z - 1].y = vertices[x][z].y;
		//if (vertices[x - 1][z - 1].y < vertices[x][z].y)
		//	vertices[x - 1][z - 1].y = vertices[x][z].y;

		//vertices[x][z].y += 0.1f;
		//for (int ii = x - 2; ii < x + 3; ii++) {
		//	for (int jj = z - 2; jj < z + 3; jj++) {
		//		vertices[ii][jj].y += 0.1f;
		//	}
		//}

		float total = 0;
		int count = 0;
		for (int ii = x - 1; ii < x + 2; ii++) {
			for (int jj = z - 1; jj < z + 2; jj++) {
				total += vertices[ii][jj].y;
				count++;
			}
		}
		total /= count;

		for (int ii = x - 1; ii < x + 1; ii++) {
			for (int jj = z - 1; jj < z + 1; jj++) {
				if(vertices[ii][jj].y <= total + 0.1)
				vertices[ii][jj].y += gain;
				vertices[ii][jj].y += 0.1;

				vertices[ii][jj].y = clamp(vertices[ii][jj].y, -0.1f, 200.0f);
			}
		}
	}
}

void Accumulation::RemovePoint(vec3 _newPoint, vec3 _scale)
{
	int x = _newPoint.x;
	int z = _newPoint.z;
	int remainder = x % segmentLength;
	if (!remainder == 0)
		x = x + segmentLength - remainder;

	remainder = z % segmentLength;
	if (!remainder == 0)
		z = z + segmentLength - remainder;
	if(x < 200 && x > 0 && z < 200 && z > 0)
	if (_newPoint.y < vertices[x][z].y) //if we're in the snow
	{
		for (int ii = x - 1; ii < x + _scale.z; ii++) {
			for (int jj = z - 1; jj < z + _scale.z + 1; jj++) {
					vertices[ii][jj].y += loss;
					vertices[ii][jj].y = clamp(vertices[ii][jj].y, 0.0f, 200.0f);
			}
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	}
}

void Accumulation::Melt()
{
	vec3 tempVertices[201][201];
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			tempVertices[i][j] = vertices[i][j];
		}
	}

	for (int i = 2; i < height-2; i++) {
		for (int j = 2; j < width-2; j++) {

			int total = 0;
			int count = 0;
			for (int ii = i - 1; ii < i+2; ii++) {
				for (int jj = j - 1; jj < j+2; jj++) {
					total += tempVertices[ii][jj].y;
					count++;
				}
			}
			total /= count;

			if (tempVertices[i][j].y > total)
			{
				int diff = tempVertices[i][j].y - total;

				vertices[i][j].y -= diff/10;
				vertices[i][j].y -= 0.001f;
				vertices[i][j].y = clamp(vertices[i][j].y, -0.1f, 200.0f);
			}
		}
	}
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
}



Accumulation::~Accumulation()
{
}

void Accumulation::Update()
{
}

void Accumulation::Draw()
{
	glUniform3f(ColourID, 0.6f, 0.16f, 0.16f);

	//glEnableVertexAttribArray(0);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	//glUniform3f(ColourID, 0.0f, 0.0f, 0.0f);
	//for (int i = 0; i < 101; i++) {
	//	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//	glDrawArrays(GL_LINE_STRIP, 101 * i, 101);
	//}

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, surface_ibo);
	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);
	glDrawElements(GL_TRIANGLES, 200 * 200 * 6, GL_UNSIGNED_SHORT, 0);

	//glUniform3f(ColourID, 1.0f, 1.0f, 1.0f);
	//glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo[2]);
	//glDrawElements(GL_TRIANGLES, 100 * 100 * 6, GL_UNSIGNED_SHORT, 0);

	////glDrawArrays(GL_LINES, 0, gridPoints.size());
	////glUniform3f(ColourID, 1.0f, 1.0f, 1.0f);
	////glDrawArrays(GL_TRIANGLE_STRIP, 0, gridPoints.size());
	glDisableVertexAttribArray(0);
}
struct Triangle {
	vec3 p1;
	vec3 p2;
	vec3 p3;

	Triangle(vec3 _p1, vec3 _p2, vec3 _p3) : p1(_p1), p2(_p2), p3(_p3) {};
};
struct Edge {
	vec3 e1;
	vec3 e2;
	Edge(vec3 e1, vec3 e2) : e1(e1), e2(e2) {};

};
vector<Edge> edgeList;
vector<Triangle> triangleList;

void SwapTest(Edge ab)
{

}

int Accumulation::RoundUp(int toRound)
{
	return (10 - toRound % 10) + toRound;
}

int Accumulation::RoundDown(int toRound)
{
	return toRound - toRound % 10;
}

void Accumulation::ResetAccumulation()
{
	for (int i = 0; i < 201; i++) {
		for (int j = 0; j < 201; j++) {
			vertices[i][j].x = i;
			vertices[i][j].y = initHeight;
			vertices[i][j].z = j;
		}
	}

	glBufferData(GL_ARRAY_BUFFER, sizeof vertices, vertices, GL_STATIC_DRAW);

}

