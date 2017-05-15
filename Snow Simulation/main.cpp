#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <gl\glut.h>
#include <gl\freeglut.h>
#include <GL\freeglut.h>
#include <GL/freeglut_ext.h>
#include <GLFW\glfw3.h>
#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <soil.h>
#include <assimp/types.h>
#include <random> // rand is biased to lower numbers

#include <AntTweakBar.h>

GLuint WINDOW_WIDTH = 1024, WINDOW_HEIGHT = 768;

const int NUMSNOWFLAKES = 1023;
const int kBoundsHeight = 201;
const int kBoundsWidth = 201;

std::random_device dseeder;
std::mt19937 rng(dseeder());
std::uniform_real_distribution<double> genDub(0, 1); //(min, max)

using namespace glm;

#include <sstream> //timer

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include "Controls.h"
#include "Shader.h"
#include "Model.h"
#include "GroundPlane.h"
#include "Accumulation.h"
#include "Deformation.h"

GLFWwindow* window; // (In the accompanying source code, this variable is global)
TwBar *bar;         // Pointer to a tweak bar

double calcFPS(double timeInterval = 1.0, std::string windowTitle = "NONE") //http://r3dux.org/2012/07/a-simple-glfw-fps-counter/
{
	// Static values which only get initialised the first time the function runs
	static double startTime = glfwGetTime(); // Set the initial time to now
	static double fps = 0.0;           // Set the initial FPS value to 0.0

									   // Set the initial frame count to -1.0 (it gets set to 0.0 on the next line). Because
									   // we don't have a start time we simply cannot get an accurate FPS value on our very
									   // first read if the time interval is zero, so we'll settle for an FPS value of zero instead.
	static double frameCount = -1.0;

	// Here again? Increment the frame count
	frameCount++;

	// Ensure the time interval between FPS checks is sane (low cap = 0.0 i.e. every frame, high cap = 10.0s)
	if (timeInterval < 0.0)
	{
		timeInterval = 0.0;
	}
	else if (timeInterval > 10.0)
	{
		timeInterval = 10.0;
	}

	// Get the duration in seconds since the last FPS reporting interval elapsed
	// as the current time minus the interval start time
	double duration = glfwGetTime() - startTime;

	// If the time interval has elapsed...
	if (duration > timeInterval)
	{
		// Calculate the FPS as the number of frames divided by the duration in seconds
		fps = frameCount / duration;

		// If the user specified a window title to append the FPS value to...
		if (windowTitle != "NONE")
		{
			// Convert the fps value into a string using an output stringstream
			std::ostringstream stream;
			stream << fps;
			std::string fpsString = stream.str();

			// Append the FPS value to the window title details
			windowTitle += " | FPS: " + fpsString;

			// Convert the new window title to a c_str and set it
			const char* pszConstString = windowTitle.c_str();
			glfwSetWindowTitle(window, pszConstString);
		}
		else // If the user didn't specify a window to append the FPS to then output the FPS to the console
		{
			std::cout << "FPS: " << fps << std::endl;
		}

		// Reset the frame count to zero and set the initial time to be now
		frameCount = 0.0;
		startTime = glfwGetTime();
	}

	// Return the current FPS - doesn't have to be used if you don't want it!
	return fps;
}



int main()
{
	/////////////////////////////////////////////////////////////////////
	// Window Creation
	/////////////////////////////////////////////////////////////////////

	if (!glfwInit())
	{
		fprintf(stderr, "Failed to initialize GLFW\n");
		return -1;
	}
	//glutInit(&myargc, myargv);
	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); //We don't want the old OpenGL 

	// Open a window and create its OpenGL context
	window = glfwCreateWindow(1024, 768, "OpenGL GLFW Window", NULL, NULL);
	if (window == NULL) 
	{
		fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); 
	glfwSetWindowPos(window, 900, 100);
	
	glewExperimental = true; // Needed in core profile
	if (glewInit() != GLEW_OK) // Initialize GLEW
	{
		fprintf(stderr, "Failed to initialize GLEW\n");

		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetScrollCallback(window, scroll_callback);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	/////////////////////////////////////////////////////////////////////
	// GUI
	/////////////////////////////////////////////////////////////////////

	// Initialize AntTweakBar
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	double time = 0;

	// Create a tweak bar
	bar = TwNewBar("TweakBar");
	TwDefine("TweakBar size='250 350'"); // resize bar

	// Add 'speed' to 'bar': it is a modifable (RW) variable of type TW_TYPE_DOUBLE. Its key shortcuts are [s] and [S].
	TwAddVarRW(bar, "WindSpeed", TW_TYPE_DOUBLE, &windSpeed, "");
	TwAddVarRW(bar, "SnowOn", TW_TYPE_BOOLCPP, &isSnowOn, "");
	TwAddVarRW(bar, "MeltOn", TW_TYPE_BOOLCPP, &isMeltOn, "");
	TwAddVarRW(bar, "GridOn", TW_TYPE_BOOLCPP, &isDebugLines, "");
	TwAddVarRW(bar, "CursorLockOn", TW_TYPE_BOOLCPP, &isCursorLocked, "");
	TwAddVarRW(bar, "SecondMarkerOn", TW_TYPE_BOOLCPP, &secondMarker, "");
	TwAddVarRO(bar, "time", TW_TYPE_DOUBLE, &time, " label='Time' precision=1 help='Time (in seconds).' ");

	TwAddSeparator(bar, "sep", "");

	TwAddButton(bar, "Controls", NULL, NULL, "");
	TwAddButton(bar, "Move W,A,S,D", NULL, NULL, "");
	TwAddButton(bar, "Height Q,E", NULL, NULL, "");
	TwAddButton(bar, "Deformation Marker", NULL, NULL, "");
	TwAddButton(bar, "    NUMPAD 2,4,6,8", NULL, NULL, "");
	TwAddButton(bar, "    Height 7,9", NULL, NULL, "");
	TwAddButton(bar, "Snow 1/2, On/Off", NULL, NULL, "");
	TwAddButton(bar, "Melt 3/4, On/Off", NULL, NULL, "");
	TwAddButton(bar, "Grid 9/0, On/Off", NULL, NULL, "");
	TwAddButton(bar, "Cursor Lock Toggle, CTRL", NULL, NULL, "");
	TwAddButton(bar, "Toggle 2nd Marker", NULL, NULL, "");
	TwAddButton(bar, "	  NUMPAD 1/3, On/Off", NULL, NULL, "");

	//// Set GLFW event callbacks
	//// - Directly redirect GLFW mouse button events to AntTweakBar
	//glfwSetMouseButtonCallback(window, (GLFWmousebuttonfun)TwEventMouseButtonGLFW);
	//// - Directly redirect GLFW mouse position events to AntTweakBar
	//glfwSetCursorPosCallback(window,(GLFWcursorposfun)TwMouseMotion);
	//// - Directly redirect GLFW mouse wheel events to AntTweakBar
	//glfwSetScrollCallback(window, (GLFWscrollfun)TwMouseWheel);
	//// - Directly redirect GLFW key events to AntTweakBar
	//glfwSetKeyCallback(window, (GLFWkeyfun)TwEventKeyGLFW);
	//// - Directly redirect GLFW char events to AntTweakBar
	//glfwSetCharCallback(window, (GLFWcharfun)TwEventCharGLFW);

	/////////////////////////////////////////////////////////////////////
	//	Shaders
	/////////////////////////////////////////////////////////////////////
	Shader colourShader("SimpleVertexShader.vertexshader", "SimpleFragmentShader.fragmentshader");
	GLuint colourShaderModelID = glGetUniformLocation(colourShader.Program, "model");
	GLuint colourShaderViewID = glGetUniformLocation(colourShader.Program, "view");
	GLuint colourShaderProjectionID = glGetUniformLocation(colourShader.Program, "projection");
	GLuint colourShaderColourID = glGetUniformLocation(colourShader.Program, "userColour");
	glUniform3f(colourShaderColourID, 0.0f, 0.0f, 0.0f);

	Shader heightShader("HeightVertexShader.vertexshader", "HeightFragmentShader.fragmentshader");
	GLuint heightShaderModelID = glGetUniformLocation(heightShader.Program, "model");
	GLuint heightShaderViewID = glGetUniformLocation(heightShader.Program, "view");
	GLuint heightShaderProjectionID = glGetUniformLocation(heightShader.Program, "projection");
	GLuint heightShaderColourID = glGetUniformLocation(heightShader.Program, "userColour");
	glUniform3f(heightShaderColourID, 0.0f, 0.0f, 0.0f);

	Shader textureShader("TexturedVertShader.vertexshader", "TexturedFragShader.fragmentshader");
	GLuint texShaderModelID = glGetUniformLocation(textureShader.Program, "model");
	GLuint texShaderViewID = glGetUniformLocation(textureShader.Program, "view");
	GLuint texShaderProjectionID = glGetUniformLocation(textureShader.Program, "projection");

	Shader instancedShader("InstancedVertShader.vertexshader", "InstancedFragShader.fragmentshader");
	GLuint Matrices_binding = 0;
	GLuint uniform_block_index = glGetUniformBlockIndex(instancedShader.Program, "Matrices");
	glUniformBlockBinding(instancedShader.Program, uniform_block_index, Matrices_binding);

	// create uniform buffer
	GLuint ubo;
	glGenBuffers(1, &ubo);
	glBindBuffer(GL_UNIFORM_BUFFER, ubo);
	glBufferData(GL_UNIFORM_BUFFER, (NUMSNOWFLAKES+1) * sizeof(float) * 4 * 4, 0, GL_STREAM_DRAW);


	/////////////////////////////////////////////
	//	SNOWFLAKES
	/////////////////////////////////////////////
	// fill the Model matrix array
	glm::mat4 ModelMatrices[NUMSNOWFLAKES];
	vector<vec3> positions;
	vector<vec3> masses;

	for (int i = 0; i < NUMSNOWFLAKES; i++)
	{
		GLfloat x = rand() % (kBoundsHeight - 4) + 2;
		GLfloat y = rand() % 200;
		GLfloat z = rand() % (kBoundsWidth - 4) + 2;
		positions.push_back(vec3(x, y, z));
		ModelMatrices[i] = glm::translate(glm::mat4(1.0f), positions[i]);

		vec3 mass = vec3(0.1f, genDub(rng), 0.1f);
		masses.push_back(mass);
	}

	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * 4, NUMSNOWFLAKES * sizeof(float) * 4 * 4, ModelMatrices);

	/////////////////////////////////////////////
	//	INSTANCING
	/////////////////////////////////////////////
	// vao and vbo handle
	GLuint vao, vbo, ibo;

	Model* snowflake = new Model("assets/snowflake.obj");
	//GLuint vao, vbo, ibo;
	// generate and bind the vao
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// generate and bind the vertex buffer object
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);

	// fill with data
	glBufferData(GL_ARRAY_BUFFER, snowflake->meshes[0].vertices.size() * sizeof(Vertex), &snowflake->meshes[0].vertices[0], GL_STATIC_DRAW);

	// Set the vertex attribute pointers
	// Vertex Positions
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);
	// Vertex Normals
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));
	// Vertex Texture Coords
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));
	// Vertex Tangent
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Tangent));
	// Vertex Bitangent
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Bitangent));

	// generate and bind the index buffer object
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);

	// fill with data
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, snowflake->meshes[0].indices.size() * sizeof(GLuint), &snowflake->meshes[0].indices[0], GL_STATIC_DRAW);

	/////////////////////////////////////////////////////////////////////
	// Object  Initialissation
	/////////////////////////////////////////////////////////////////////

	////INSTANCING
	instancedShader.Use();

	Model snowTest("assets/snowflake.obj");

	//GroundPlane* groundPlane = new GroundPlane(colourShader.Program); //for some reason this has to be after the Models are loaded

	/////////////////////////////////////////////////////////////////////
	// Lighting
	/////////////////////////////////////////////////////////////////////
	glm::vec3 lightPos(0.0f, 0.0f, 0.0f);

	/////////////////////////////////////////////////////////////////////
	// Tesselation
	/////////////////////////////////////////////////////////////////////
	/*
	GLint MaxPatchVertices = 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
	printf("Max supported patch vertices %d\n", MaxPatchVertices); //32 on Mellor PCs
	glPatchParameteri(GL_PATCH_VERTICES, 3);
	*/

	//https://en.wikipedia.org/wiki/Delaunay_triangulation
	//https://github.com/Bl4ckb0ne/delaunay-triangulation	
	//http://stackoverflow.com/questions/11634581/tessellate-a-plane-of-points
	Accumulation* acm = new Accumulation(colourShader.Program);

	/////////////////////////////////////////////////////////////////////
	// Deformation
	/////////////////////////////////////////////////////////////////////
	//http://www.wildbunny.co.uk/blog/2011/04/20/collision-detection-for-dummies/
	//Deformation dfm(colourShader.Program);

	/////////////////////////////////////////////////////////////////////
	// Game Loop
	/////////////////////////////////////////////////////////////////////
	glfwSetCursorPos(window, 1024 / 2, 768 / 2);


	/////////////////////////////////////////////////////////////////////
	// Threading
	/////////////////////////////////////////////////////////////////////
	//vector<std::thread> threads;

	/////////////////////////////////////////////////////////////////////
	// Melting
	/////////////////////////////////////////////////////////////////////
	int meltTimer = 0;
	int meltTime = 5;

	/////////////////////////////////////////////////////////////////////
	// Testing
	/////////////////////////////////////////////////////////////////////
	double lastTime = glfwGetTime();
	int nbFrames = 0;

	ofstream myfile;
	vector<string> vec_times;
	vector<string> vec_windSpeed;
	vector<string> vec_snowing;
	vector<string> vec_melting;
	bool forceClose = false;

	do {
		/////////////////////////////////////////////////////////////////////
		// Testing
		/////////////////////////////////////////////////////////////////////
		/*
		double currentTime = glfwGetTime();
		nbFrames++;
		if (currentTime - lastTime >= 1.0) { // If last prinf() was more than 1 sec ago
		// printf and reset timer
		printf("%f ms/frame\n", 1000.0 / double(nbFrames));

		vec_times.push_back(to_string(1000.0 / static_cast<double>(nbFrames)));
		vec_windSpeed.push_back(to_string(windSpeed));
		vec_snowing.push_back(to_string(isSnowOn));
		vec_melting.push_back(to_string(isMeltOn));

		if (glfwGetTime() > 600)
		forceClose = true;

		nbFrames = 0;
		lastTime += 1.0;
		}
		*/

		// clear screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		float t = glfwGetTime();
		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs(window);

		///////////////////////////////////////////////////////////////////////
		// UPDATE POSITIONS
		///////////////////////////////////////////////////////////////////////
		if (resetAccumulation)
		{
			acm->ResetAccumulation();
			resetAccumulation = false;
		}
		
		//*************************************//
		// Deformation
		//*************************************//
		acm->RemovePoint(getControlledCubePostionVec(), controlledCubeScale);
		vec3 secondTrack = getControlledCubePostionVec();
		secondTrack.z -= 20;
		if (secondMarker)
			acm->RemovePoint(secondTrack, controlledCubeScale);

		//*************************************//
		// Melting
		//*************************************//
		if (isMeltOn)
		{
			meltTimer += t;

			if (meltTimer > meltTime)
			{
				acm->Melt();

				meltTimer = 0;
			}
		}

		//*************************************//
		// Snowflakes
		//*************************************//
		if (isSnowOn)
		{
			//https://gamedevelopment.tutsplus.com/tutorials/how-to-create-a-custom-2d-physics-engine-the-basics-and-impulse-resolution--gamedev-6331
			for (int i = 0; i < NUMSNOWFLAKES; i++)
			{
				//*************************************//
				// Falling Dynamics
				//*************************************//
				vec3 gravity = vec3(1.0f, -9.81f, 1.0f);
				vec3 wind = vec3(1.0f, windSpeed, 1.0f);
				vec3 force = masses[i] * (gravity * wind);
				positions[i].y += force.y;
				positions[i].x += force.x;
				positions[i].z += force.z;

				//*************************************//
				// Accumulation
				//*************************************//
				if (positions[i].y < 0)
				{
					acm->AddPoint(positions[i]);
					positions[i].x = rand() % (kBoundsHeight - 2) + 2;
					positions[i].y = rand() % 200;
					positions[i].z = rand() % (kBoundsWidth - 2) + 2;
				}
				ModelMatrices[i] = glm::translate(mat4(1.0f), positions[i]);
				ModelMatrices[i] = glm::scale(ModelMatrices[i], vec3(0.01f, 0.01f, 0.01f));
				ModelMatrices[i] = glm::rotate(ModelMatrices[i], t+i, vec3(0.1f, 0.1f, 0.1f));
			}
			//cout << positions[0].x << "	" << positions[0].y << "	" << positions[0].z << "	" << endl << endl;
			glBufferSubData(GL_UNIFORM_BUFFER, sizeof(float) * 4 * 4, NUMSNOWFLAKES * sizeof(float) * 4 * 4, ModelMatrices);
		}


		///////////////////////////////////////////////////////////////////////
		//// DRAW
		///////////////////////////////////////////////////////////////////////
		//http://roxlu.com/2014/028/opengl-instanced-rendering

		// calculate ViewProjection matrix
		glm::mat4 Projection = getProjectionMatrix();
		// translate the world/view position
		glm::mat4 View = getViewMatrix();
		glm::mat4 ViewProjection = Projection*View;
		glm::mat4 model = glm::mat4(1.0);

		//*************************************//
		// Snowflakes
		//*************************************//
		if (isSnowOn)
		{
			// use the shader program
			instancedShader.Use();
			glUniform3f(colourShaderColourID, 0.0f, 0.0f, 0.0f);

			// set the ViewProjection in the uniform buffer
			glBindBuffer(GL_UNIFORM_BUFFER, ubo);
			glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(float) * 4 * 4, glm::value_ptr(ViewProjection));
			glBindBufferRange(GL_UNIFORM_BUFFER, Matrices_binding, ubo, 0, sizeof(float) * 4 * 4 * NUMSNOWFLAKES);
			// bind the vao
			glBindVertexArray(vao);
			// the additional parameter indicates how many instances to render
			glDrawElementsInstanced(GL_TRIANGLES, snowflake->meshes[0].indices.size(), GL_UNSIGNED_INT, 0, NUMSNOWFLAKES);
			glBindVertexArray(0);
		}

		//*************************************//
		// Deformation
		//*************************************//
		// Use our shader
		colourShader.Use();
		ProjectionMatrix = getProjectionMatrix();
		ViewMatrix = getViewMatrix();
		glUniformMatrix4fv(colourShaderProjectionID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
		glUniformMatrix4fv(colourShaderViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
		model = glm::mat4(1.0);
		glUniformMatrix4fv(colourShaderModelID, 1, GL_FALSE, &model[0][0]);
		GroundPlane gp(colourShader.Program);
		gp.Draw();
		Deformation* dfm = new Deformation(colourShader.Program);
		dfm->scale = controlledCubeScale;
		dfm->Draw(getControlledCubePostionVec());
		if (secondMarker)
			dfm->Draw(secondTrack);

		//*************************************//
		// Accumulation
		//*************************************//
		heightShader.Use();
		glUniformMatrix4fv(heightShaderProjectionID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
		glUniformMatrix4fv(heightShaderViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
		model = glm::mat4(1.0);
		glUniformMatrix4fv(heightShaderModelID, 1, GL_FALSE, &model[0][0]);
		acm->Draw();


		//*************************************//
		// Debug Misc
		//*************************************//

		if (isDebugLines)
		{
			colourShader.Use();
			ProjectionMatrix = getProjectionMatrix();
			ViewMatrix = getViewMatrix();
			glUniformMatrix4fv(colourShaderProjectionID, 1, GL_FALSE, &ProjectionMatrix[0][0]);
			glUniformMatrix4fv(colourShaderViewID, 1, GL_FALSE, &ViewMatrix[0][0]);
			model = glm::mat4(1.0);
			glUniformMatrix4fv(colourShaderModelID, 1, GL_FALSE, &model[0][0]);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			acm->Draw();
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}

		// Draw tweak bars
		colourShader.Use();
		time = glfwGetTime();
		TwRefreshBar(bar);
		TwDraw();

		// Swap buffers
		glfwSwapBuffers(window); // REDRAW
		glfwPollEvents(); // INPUT
		calcFPS(1.0, "FPS Counter: ");
		//system("cls");

	} // Check if the ESC key was pressed or the window was closed
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0 && forceClose == false);

	///////////////////////////////////////////////////////////////////////
	//// Benchmark Testing
	///////////////////////////////////////////////////////////////////////
	/*
	stringstream strs;
	strs << NUMSNOWFLAKES << " Snow_" << isSnowOn << " Melt_" << isMeltOn << " WindSpeed_" << windSpeed << ".csv";
	ofstream ofs;
	ofs.open(strs.str());
	ofs << "UpdateTime,";
	for each (string str in vec_times)
	{
		ofs << str << ",";
	}
	ofs << "\n";
	ofs << "Snowing,";
	for each (string str in vec_snowing)
	{
		ofs << str << ",";
	}
	ofs << "\n";
	ofs << "Melting,";
	for each (string str in vec_melting)
	{
		ofs << str << ",";
	}
	ofs << "\n";
	ofs << "Wind Speed,";
	for each (string str in vec_windSpeed)
	{
		ofs << str << ",";
	}
	ofs << "\n";
	*/

	// Cleanup VBO and shader
	glDeleteProgram(colourShader.Program);
	glDeleteProgram(textureShader.Program);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();
	TwTerminate();
}

