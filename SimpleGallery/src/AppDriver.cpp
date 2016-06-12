#include <GL/glew.h>
#include <GL/freeglut.h>
#include <IL/il.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"
#include "Model.h"
#include "Window.h"

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;
const std::string WINDOW_TITLE = "SimpleGallery";

// Models used
Model maze, ground, fan, pedestal, table, vases,
      portrait, benches, ceilingLamp, portraits,
      star, pie, pentCrystal, pentPrism;

// Shader settings
// Uniform binding points
GLuint matricesUniLoc = 1, materialUniLoc = 2;
GLuint texUnit = 0;
Shader shader;

// Frame counting and FPS computation
long time, timebase = 0, frame = 0;
std::string frameRateText;

std::string timeOfDay = "Day time";
std::string displayState;

Window mainWindow(WINDOW_WIDTH, WINDOW_HEIGHT);
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;

// Coordinates taken from Blender
const int NUM_OF_POINT_LIGHTS = 9;
const GLfloat pointLightY = 5.54441f;
const GLfloat pointLightLocations[NUM_OF_POINT_LIGHTS][2] = {
	// x and z only
	{0.0f, 0.0f},
	{13.0f, 13.0f},
	{13.0f, 0.0f},
	{13.0f, -13.0f},
	{0.0f, 13.0f},
	{0.0f, -13.0f},
	{-13.0f, -13.0f},
	{-13.0f, 0.0f},
	{-13.0f, 13.0f}
};

const int NUM_OF_PEDESTALS = 36;
const GLfloat pedestalY = 0.41272f;
const GLfloat pedestalLocations[NUM_OF_PEDESTALS][2] = {
	// x and z only
	// Middle room
	{5.0f, 5.0f},
	{5.0f, -5.0f},
	{-5.0f, 5.0f},
	{-5.0f, -5.0f},

	// StarCraft room
	{5.0f, -8.0f},
	{5.0f, -18.0f},
	{-5.0f, -8.0f},
	{-5.0f, -18.0f},

	// Minecraft room
	{5.0f, 8.0f},
	{5.0f, 18.0f},
	{-5.0f, 8.0f},
	{-5.0f, 18.0f},

	// HotS room
	{18.0f, 18.0f},
	{18.0f, 8.0f},
	{8.0f, 8.0f},
	{8.0f, 18.0f},

	// Overwatch room
	{18.0f, 5.0f},
	{8.0f, 5.0f},
	{18.0f, -5.0f},
	{8.0f, -5.0f},

	// Tomb raider room
	{18.0f, -8.0f},
	{8.0f, -8.0f},
	{18.0f, -18.0f},
	{8.0f, -18.0f},

	// Dota room
	{-18.0f, -18.0f},
	{-18.0f, -8.0f},
	{-8.0f, -18.0f},
	{-8.0f, -8.0f},

	// Diablo room
	{-18.0f, -5.0f},
	{-18.0f, 5.0f},
	{-8.0f, -5.0f},
	{-8.0f, 5.0f},

	// Crysis room
	{-18.0f, 8.0f},
	{-18.0f, 18.0f},
	{-8.0f, 8.0f},
	{-8.0f, 18.0f},
};

void keyCallback(unsigned char key, int x, int y);
void keyUpCallback(unsigned char key, int x, int y);
void specialCallback(int key, int x, int y);
void specialUpCallback(int key, int x, int y);
void idleCallback();

void reshapeCallback(int w, int h)
{
	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).
	if (h == 0)
		h = 1;

	// Set the viewport to be the entire window
	glViewport(0, 0, w, h);
}

void RenderModel(const Model& model, const aiNode* nd)
{
	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	mainWindow.ctm.PushMatrix();

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);

	mainWindow.ctm.MultMatrix(aux);
	mainWindow.ctm.SetModel();

	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
	{
		if (mainWindow.drawingMode == DrawingMode::WIREFRAME)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, materialUniLoc, mainWindow.currentMatId, 0, sizeof(Material));
		}
		else
		{
			switch (mainWindow.solidMode)
			{
			case SolidMode::BASIC:
			case SolidMode::LIGHTINGONLY:
				// bind material uniform
				glBindBufferRange(GL_UNIFORM_BUFFER, materialUniLoc, model.meshes[nd->mMeshes[n]].uniformBlockIndex, 0, sizeof(Material));
				break;
			default:
				// bind material uniform
				glBindBufferRange(GL_UNIFORM_BUFFER, materialUniLoc, model.meshes[nd->mMeshes[n]].uniformBlockIndex, 0, sizeof(Material));
				// bind texture
				glBindTexture(GL_TEXTURE_2D, model.meshes[nd->mMeshes[n]].texIndex);
				break;
			}
		}

		// bind VAO
		glBindVertexArray(model.meshes[nd->mMeshes[n]].vao);
		glDrawElements(GL_TRIANGLES, model.meshes[nd->mMeshes[n]].numFaces * 3, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n)
	{
		RenderModel(model, nd->mChildren[n]);
	}

	mainWindow.ctm.PopMatrix();
}

void RenderWithTex(const Model& model, const aiNode* nd, const GLuint& texId)
{
	// Get node transformation matrix
	aiMatrix4x4 m = nd->mTransformation;
	// OpenGL matrices are column major
	m.Transpose();

	// save model matrix and apply node transformation
	mainWindow.ctm.PushMatrix();

	float aux[16];
	memcpy(aux, &m, sizeof(float) * 16);

	mainWindow.ctm.MultMatrix(aux);
	mainWindow.ctm.SetModel();

	// draw all meshes assigned to this node
	for (unsigned int n = 0; n < nd->mNumMeshes; ++n)
	{
		if (mainWindow.drawingMode == DrawingMode::WIREFRAME)
		{
			glBindBufferRange(GL_UNIFORM_BUFFER, materialUniLoc, mainWindow.currentMatId, 0, sizeof(Material));
		}
		else
		{
			switch (mainWindow.solidMode)
			{
			case SolidMode::BASIC:
			case SolidMode::LIGHTINGONLY:
				// bind material uniform
				glBindBufferRange(GL_UNIFORM_BUFFER, materialUniLoc, model.meshes[nd->mMeshes[n]].uniformBlockIndex, 0, sizeof(Material));
				break;
			default:
				// bind texture
				glUniform1i(glGetUniformLocation(shader(), "forceTextured"), true);
				glBindBufferRange(GL_UNIFORM_BUFFER, materialUniLoc, model.meshes[nd->mMeshes[n]].uniformBlockIndex, 0, sizeof(Material));
				glBindTexture(GL_TEXTURE_2D, texId);
				break;
			}
		}

		// bind VAO
		glBindVertexArray(model.meshes[nd->mMeshes[n]].vao);
		glDrawElements(GL_TRIANGLES, model.meshes[nd->mMeshes[n]].numFaces * 3, GL_UNSIGNED_INT, nullptr);
		glBindVertexArray(0);
		glBindTexture(GL_TEXTURE_2D, 0);
		glUniform1i(glGetUniformLocation(shader(), "forceTextured"), false);
	}

	// draw all children
	for (unsigned int n = 0; n < nd->mNumChildren; ++n)
	{
		RenderWithTex(model, nd->mChildren[n], texId);
	}

	mainWindow.ctm.PopMatrix();
}

void PrintText(const GLfloat& x, const GLfloat& y, void* font, const char* const str)
{
	const char* ptr; // Temp pointer to position in string

	glRasterPos2f(x, y); // Place raster position

	for (ptr = str; *ptr != '\0'; ++ptr)
	{
		glutBitmapCharacter(font, *ptr); // Draw bitmap character
	}
}

void renderHelpInstructions()
{
	// Note: This uses compatibility mode to render bitmap characters
	// TODO Implement freetype inplace of glutBitmapCharacter
	glUseProgram(0); // Use fixed function shader
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluOrtho2D(0, 800, 0, 600); // left, right, down, up
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	timeOfDay = "Time of day: " + std::string(mainWindow.timeOfDay ? "Day" : "Night") + " time";
	displayState = mainWindow.GetDisplayStateString();

	glColor3f(1.0f, 0.0f, 0.0f);
	PrintText(10, 580, GLUT_BITMAP_HELVETICA_12, frameRateText.c_str());
	PrintText(10, 560, GLUT_BITMAP_HELVETICA_12, timeOfDay.c_str());
	PrintText(10, 540, GLUT_BITMAP_HELVETICA_12, displayState.c_str());
	PrintText(10, 520, GLUT_BITMAP_HELVETICA_12, "----- Camera controls -----");
	PrintText(10, 500, GLUT_BITMAP_HELVETICA_12, "w - Move forward");
	PrintText(10, 480, GLUT_BITMAP_HELVETICA_12, "a - Move to the left");
	PrintText(10, 460, GLUT_BITMAP_HELVETICA_12, "s - Move backward");
	PrintText(10, 440, GLUT_BITMAP_HELVETICA_12, "d - Move to the right");
	PrintText(10, 420, GLUT_BITMAP_HELVETICA_12, "q - Roll left");
	PrintText(10, 400, GLUT_BITMAP_HELVETICA_12, "e - Roll right");
	PrintText(10, 380, GLUT_BITMAP_HELVETICA_12, "PAGE UP - Move upward");
	PrintText(10, 360, GLUT_BITMAP_HELVETICA_12, "PAGE DOWN - Move downward");
	PrintText(10, 340, GLUT_BITMAP_HELVETICA_12, "HOME - Zoom in");
	PrintText(10, 320, GLUT_BITMAP_HELVETICA_12, "END - Zoom out");
	PrintText(10, 300, GLUT_BITMAP_HELVETICA_12, "UP - Look up");
	PrintText(10, 280, GLUT_BITMAP_HELVETICA_12, "DOWN - Look down");
	PrintText(10, 260, GLUT_BITMAP_HELVETICA_12, "LEFT - Look left");
	PrintText(10, 240, GLUT_BITMAP_HELVETICA_12, "RIGHT - Look right");
	PrintText(10, 220, GLUT_BITMAP_HELVETICA_12, "0 - Reset camera position");
	PrintText(10, 200, GLUT_BITMAP_HELVETICA_12, "----- Drawing mode controls -----");
	PrintText(10, 180, GLUT_BITMAP_HELVETICA_12, "z - Wireframe black/white");
	PrintText(10, 160, GLUT_BITMAP_HELVETICA_12, "x - Wireframe white/black");
	PrintText(10, 140, GLUT_BITMAP_HELVETICA_12, "c - Wireframe blue/yellow");
	PrintText(10, 120, GLUT_BITMAP_HELVETICA_12, "v - Solid colors");
	PrintText(10, 100, GLUT_BITMAP_HELVETICA_12, "b - Solid lighting");
	PrintText(10, 80, GLUT_BITMAP_HELVETICA_12, "n - Solid texture");
	PrintText(10, 60, GLUT_BITMAP_HELVETICA_12, "m - Solid smooth shading");
	PrintText(310, 520, GLUT_BITMAP_HELVETICA_12, "----- Flashlight controls -----");
	PrintText(310, 500, GLUT_BITMAP_HELVETICA_12, "f - On/Off flashlight");
	PrintText(310, 480, GLUT_BITMAP_HELVETICA_12, "j - Increase intensity");
	PrintText(310, 460, GLUT_BITMAP_HELVETICA_12, "k - Decrease intensity");
	PrintText(310, 440, GLUT_BITMAP_HELVETICA_12, "l - Cycle between R, G, B color");
	PrintText(310, 420, GLUT_BITMAP_HELVETICA_12, "----- Other controls -----");
	PrintText(310, 400, GLUT_BITMAP_HELVETICA_12, "h - Toggle this help text");
	PrintText(310, 380, GLUT_BITMAP_HELVETICA_12, "p - Take screenshot");
	PrintText(310, 360, GLUT_BITMAP_HELVETICA_12, "o - Toggle anti aliasing");
	PrintText(310, 340, GLUT_BITMAP_HELVETICA_12, "t - Toggle translucent surfaces");
	PrintText(310, 320, GLUT_BITMAP_HELVETICA_12, "i - Toggle day/night");
	PrintText(310, 300, GLUT_BITMAP_HELVETICA_12, "ESC - Quit");
	PrintText(610, 520, GLUT_BITMAP_HELVETICA_12, "----- Light controls -----");
	PrintText(610, 500, GLUT_BITMAP_HELVETICA_12, "1 - Toggle light 1");
	PrintText(610, 480, GLUT_BITMAP_HELVETICA_12, "2 - Toggle light 2");
	PrintText(610, 460, GLUT_BITMAP_HELVETICA_12, "3 - Toggle light 3");
	PrintText(610, 440, GLUT_BITMAP_HELVETICA_12, "4 - Toggle light 4");
	PrintText(610, 420, GLUT_BITMAP_HELVETICA_12, "5 - Toggle light 5");
	PrintText(610, 400, GLUT_BITMAP_HELVETICA_12, "6 - Toggle light 6");
	PrintText(610, 380, GLUT_BITMAP_HELVETICA_12, "7 - Toggle light 7");
	PrintText(610, 360, GLUT_BITMAP_HELVETICA_12, "8 - Toggle light 8");
	PrintText(610, 340, GLUT_BITMAP_HELVETICA_12, "9 - Toggle light 9");
	PrintText(610, 320, GLUT_BITMAP_HELVETICA_12, "; - Toggle spot light 1");
	PrintText(610, 300, GLUT_BITMAP_HELVETICA_12, "' - Toggle spot light 2");
}

void SetDirLight(const Shader& shader, const int& index)
{
	const auto name = "dirLights[" + std::to_string(index) + "].";
	glUniform3f(glGetUniformLocation(shader(), (name + "direction").c_str()), 0.0f, -1.0f, 0.0f);
	glUniform3f(glGetUniformLocation(shader(), (name + "ambient").c_str()), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader(), (name + "diffuse").c_str()), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader(), (name + "specular").c_str()), 0.5f, 0.5f, 0.5f);
}

void SetPointLight(const Shader& shader, const int& index)
{
	const auto name = "pointLights[" + std::to_string(index) + "].";
	glUniform3f(glGetUniformLocation(shader(), (name + "position").c_str()), 0.0f, 0.0f, 0.0f);
	glUniform1f(glGetUniformLocation(shader(), (name + "constant").c_str()), 1.0f);
	glUniform1f(glGetUniformLocation(shader(), (name + "linear").c_str()), 0.09f);
	glUniform1f(glGetUniformLocation(shader(), (name + "quadratic").c_str()), 0.032f);
	glUniform3f(glGetUniformLocation(shader(), (name + "ambient").c_str()), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader(), (name + "diffuse").c_str()), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader(), (name + "specular").c_str()), 0.5f, 0.5f, 0.5f);
}

void SetSpotLight(const Shader& shader, const int& index)
{
	const auto name = "spotLights[" + std::to_string(index) + "].";
	glUniform3f(glGetUniformLocation(shader(), (name + "position").c_str()), 0.0f, 1.0f, 0.0f);
	glUniform3f(glGetUniformLocation(shader(), (name + "direction").c_str()), 0.0f, -1.0f, 0.0f);
	glUniform1f(glGetUniformLocation(shader(), (name + "cutOff").c_str()), glm::cos(glm::radians(52.5f)));
	glUniform1f(glGetUniformLocation(shader(), (name + "outerCutOff").c_str()), glm::cos(glm::radians(55.0f)));
	glUniform1f(glGetUniformLocation(shader(), (name + "constant").c_str()), 1.0f);
	glUniform1f(glGetUniformLocation(shader(), (name + "linear").c_str()), 0.09f);
	glUniform1f(glGetUniformLocation(shader(), (name + "quadratic").c_str()), 0.032f);
	glUniform3f(glGetUniformLocation(shader(), (name + "ambient").c_str()), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader(), (name + "diffuse").c_str()), 0.5f, 0.5f, 0.5f);
	glUniform3f(glGetUniformLocation(shader(), (name + "specular").c_str()), 0.5f, 0.5f, 0.5f);
}

void SetFlashLight(const Shader& shader, const Camera& camera, const glm::vec3& diffuse, const GLfloat& intensity)
{
	auto temp = diffuse * intensity;
	glUniform3f(glGetUniformLocation(shader(), "flashLight.position"), camera.Position.x, camera.Position.y, camera.Position.z);
	glUniform3f(glGetUniformLocation(shader(), "flashLight.direction"), camera.Front.x, camera.Front.y, camera.Front.z);
	glUniform1f(glGetUniformLocation(shader(), "flashLight.cutOff"), glm::cos(glm::radians(12.5f)));
	glUniform1f(glGetUniformLocation(shader(), "flashLight.outerCutOff"), glm::cos(glm::radians(15.0f)));
	glUniform1f(glGetUniformLocation(shader(), "flashLight.constant"), 1.0f);
	glUniform1f(glGetUniformLocation(shader(), "flashLight.linear"), 0.09f);
	glUniform1f(glGetUniformLocation(shader(), "flashLight.quadratic"), 0.032f);
	glUniform3f(glGetUniformLocation(shader(), "flashLight.ambient"), temp.x, temp.y, temp.z);
	glUniform3f(glGetUniformLocation(shader(), "flashLight.diffuse"), temp.x, temp.y, temp.z);
	glUniform3f(glGetUniformLocation(shader(), "flashLight.specular"), temp.x, temp.y, temp.z);
}

void SetNumOfDirLights(const Shader& shader, int numOfDirLights)
{
	glUniform1i(glGetUniformLocation(shader(), "numOfDirLights"), numOfDirLights);
}

void SetNumOfSpotLights(const Shader& shader, int numOfSpotLights)
{
	glUniform1i(glGetUniformLocation(shader(), "numOfSpotLights"), numOfSpotLights);
}

void SetNumOfPointLights(const Shader& shader, int numOfPointLights)
{
	glUniform1i(glGetUniformLocation(shader(), "numOfPointLights"), numOfPointLights);
}

void SetPointLightPosition(const Shader& shader, const int& index, const glm::vec3& position)
{
	const auto name = "pointLights[" + std::to_string(index) + "].";
	glUniform3f(glGetUniformLocation(shader(), (name + "position").c_str()), position.x, position.y, position.z);
}

void SetPointLightColor(const Shader& shader, const int& index, const glm::vec3& diffuse)
{
	const auto name = "pointLights[" + std::to_string(index) + "].";
	glUniform3f(glGetUniformLocation(shader(), (name + "ambient").c_str()), diffuse.x, diffuse.y, diffuse.z);
	glUniform3f(glGetUniformLocation(shader(), (name + "diffuse").c_str()), diffuse.x, diffuse.y, diffuse.z);
	glUniform3f(glGetUniformLocation(shader(), (name + "specular").c_str()), diffuse.x, diffuse.y, diffuse.z);
}

void SetSpotLightPosition(const Shader& shader, const int& index, const glm::vec3& position)
{
	const auto name = "spotLights[" + std::to_string(index) + "].";
	glUniform3f(glGetUniformLocation(shader(), (name + "position").c_str()), position.x, position.y, position.z);
}

void SetSpotLightDirection(const Shader& shader, const int& index, const glm::vec3& direction)
{
	const auto name = "spotLights[" + std::to_string(index) + "].";
	glUniform3f(glGetUniformLocation(shader(), (name + "direction").c_str()), direction.x, direction.y, direction.z);
}

void SetSpotLightColor(const Shader& shader, const int& index, const glm::vec3& diffuse)
{
	const auto name = "spotLights[" + std::to_string(index) + "].";
	glUniform3f(glGetUniformLocation(shader(), (name + "ambient").c_str()), diffuse.x, diffuse.y, diffuse.z);
	glUniform3f(glGetUniformLocation(shader(), (name + "diffuse").c_str()), diffuse.x, diffuse.y, diffuse.z);
	glUniform3f(glGetUniformLocation(shader(), (name + "specular").c_str()), diffuse.x, diffuse.y, diffuse.z);
}

void ToggleFlashLight(const Shader& shader, bool flashLightToggle)
{
	glUniform1i(glGetUniformLocation(shader(), "flashLightOn"), flashLightToggle);
}

void SetLighting(const Shader& shader, bool lightingToggle)
{
	glUniform1i(glGetUniformLocation(shader(), "lighting"), lightingToggle);
}

void displayCallback()
{
	const auto width = glutGet(GLUT_WINDOW_WIDTH);
	const auto height = glutGet(GLUT_WINDOW_HEIGHT);
	auto ratio = (1.0f * width) / height;

	shader.Use();

	mainWindow.ctm.SetPerspective(mainWindow.camera.Zoom, ratio, 0.1f, 100.0f);
	mainWindow.SetTimeOfDay();
	mainWindow.SetDrawingMode();
	mainWindow.SetAntiAliasing();
	mainWindow.SetViewMatrix(shader);
	mainWindow.SetTexture();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	SetNumOfSpotLights(shader, 2);
	SetSpotLight(shader, 0);
	SetSpotLight(shader, 1);
	SetSpotLightPosition(shader, 0, glm::vec3(20.0f * sin(glutGet(GLUT_ELAPSED_TIME) / 1000.0f), 2.0f, 0.0f));
	SetSpotLightPosition(shader, 1, glm::vec3(0.0f, 2.0f, -20.0f * sin(glutGet(GLUT_ELAPSED_TIME) / 1000.0f)));
	if (mainWindow.spotLights[0])
	{
		SetSpotLightColor(shader, 0, sin(glutGet(GLUT_ELAPSED_TIME) / 100.0f) * glm::vec3(0.0f, 1.0f, 0.0f));
	}
	else
	{
		SetSpotLightColor(shader, 0, glm::vec3(0.0f, 0.0f, 0.0f));
	}

	if (mainWindow.spotLights[1])
	{
		SetSpotLightColor(shader, 1, 10.0f * sin(glutGet(GLUT_ELAPSED_TIME) / 100.0f) * glm::vec3(1.0f, 0.0f, 0.0f));
	}
	else
	{
		SetSpotLightColor(shader, 1, glm::vec3(0.0f, 0.0f, 0.0f));
	}


	if (mainWindow.timeOfDay)
	{
		SetNumOfDirLights(shader, 1);
		for (auto i = 0; i < 1; ++i)
		{
			SetDirLight(shader, i);
		}
	}
	else
	{
		SetNumOfDirLights(shader, 0);
	}

	SetFlashLight(shader, mainWindow.camera, mainWindow.flashLightDiffuse, mainWindow.intensity);
	ToggleFlashLight(shader, mainWindow.flashLightOn);
	SetLighting(shader, mainWindow.lighting);

	glDisable(GL_BLEND);
	mainWindow.ctm.LoadIdentity();
	mainWindow.ctm.Rotate(static_cast<GLfloat>(glutGet(GLUT_ELAPSED_TIME)), glm::vec3(0.0f, 1.0f, 0.0f));
	mainWindow.ctm.SetModel();
	RenderModel(fan, fan.scene->mRootNode);

	SetNumOfPointLights(shader, NUM_OF_POINT_LIGHTS);
	for (auto i = 0; i < NUM_OF_POINT_LIGHTS; ++i)
	{
		SetPointLight(shader, i);
		SetPointLightPosition(shader, i, glm::vec3(pointLightLocations[i][0], pointLightY, pointLightLocations[i][1]));

		glDisable(GL_BLEND);
		mainWindow.ctm.LoadIdentity();
		mainWindow.ctm.Translate(pointLightLocations[i][0], 0.0f, pointLightLocations[i][1]); // y axis not needed
		mainWindow.ctm.SetModel();
		RenderModel(ceilingLamp, ceilingLamp.scene->mRootNode);

		if (mainWindow.lights[i])
		{
			SetPointLightColor(shader, i, glm::vec3(0.5f, 0.5f, 0.5f));
		}
		else
		{
			SetPointLightColor(shader, i, glm::vec3(0.0f, 0.0f, 0.0f));
		}
	}

	auto ornamentChooser = 0;
	for (auto i = 0; i < NUM_OF_PEDESTALS; ++i)
	{
		glDisable(GL_BLEND);
		mainWindow.ctm.LoadIdentity();
		mainWindow.ctm.Translate(pedestalLocations[i][0], 0.0f, pedestalLocations[i][1]);
		mainWindow.ctm.SetModel();
		RenderModel(pedestal, pedestal.scene->mRootNode);

		switch (ornamentChooser)
		{
		case 0:
			glDisable(GL_BLEND);
			mainWindow.ctm.LoadIdentity();
			mainWindow.ctm.Translate(pedestalLocations[i][0], 0.0f, pedestalLocations[i][1]);
			mainWindow.ctm.Rotate(glutGet(GLUT_ELAPSED_TIME) / 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			mainWindow.ctm.SetModel();
			RenderModel(star, star.scene->mRootNode);
			ornamentChooser = 1;
			break;
		case 1:
			glDisable(GL_BLEND);
			mainWindow.ctm.LoadIdentity();
			mainWindow.ctm.Translate(pedestalLocations[i][0], 0.0f, pedestalLocations[i][1]);
			mainWindow.ctm.Rotate(glutGet(GLUT_ELAPSED_TIME) / 10.0f, glm::vec3(0.0f, -1.0f, 0.0f));
			mainWindow.ctm.SetModel();
			RenderModel(pentCrystal, pentCrystal.scene->mRootNode);
			ornamentChooser = 2;
			break;
		case 2:
			glDisable(GL_BLEND);
			mainWindow.ctm.LoadIdentity();
			mainWindow.ctm.Translate(pedestalLocations[i][0], 0.0f, pedestalLocations[i][1]);
			mainWindow.ctm.Rotate(glutGet(GLUT_ELAPSED_TIME) / 10.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			mainWindow.ctm.SetModel();
			RenderModel(pentPrism, pentPrism.scene->mRootNode);
			ornamentChooser = 3;
			break;
		case 3:
			glDisable(GL_BLEND);
			mainWindow.ctm.LoadIdentity();
			mainWindow.ctm.Translate(pedestalLocations[i][0], 0.0f, pedestalLocations[i][1]);
			mainWindow.ctm.Rotate(glutGet(GLUT_ELAPSED_TIME) / 10.0f, glm::vec3(0.0f, -1.0f, 0.0f));
			mainWindow.ctm.SetModel();
			RenderModel(pie, pie.scene->mRootNode);
			ornamentChooser = 0;
			break;
		}
	}


	glDisable(GL_BLEND);
	mainWindow.ctm.LoadIdentity();
	mainWindow.ctm.SetModel();
	RenderModel(benches, benches.scene->mRootNode);

	glDisable(GL_BLEND);
	mainWindow.ctm.LoadIdentity();
	mainWindow.ctm.SetModel();
	RenderModel(table, table.scene->mRootNode);

	glDisable(GL_BLEND);
	mainWindow.ctm.LoadIdentity();
	mainWindow.ctm.SetModel();
	RenderModel(vases, vases.scene->mRootNode);

	glDisable(GL_BLEND);
	mainWindow.ctm.LoadIdentity();
	mainWindow.ctm.SetModel();
	RenderWithTex(portrait, portrait.scene->mRootNode, mainWindow.screenshotTexId);

	glDisable(GL_BLEND);
	mainWindow.ctm.LoadIdentity();
	mainWindow.ctm.SetModel();
	RenderModel(portraits, portraits.scene->mRootNode);

	if (mainWindow.blending)
	{
		glDisable(GL_DEPTH_TEST);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

		glDisable(GL_BLEND);
		mainWindow.ctm.LoadIdentity();
		mainWindow.ctm.SetModel();
		RenderModel(ground, ground.scene->mRootNode);

		glEnable(GL_DEPTH_TEST);
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glStencilFunc(GL_EQUAL, 1, 0xff);
		glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);

		if (mainWindow.camera.Position.y > 0.0f)
		{
			glDisable(GL_BLEND);
			mainWindow.ctm.LoadIdentity();
			mainWindow.ctm.Rotate(static_cast<GLfloat>(glutGet(GLUT_ELAPSED_TIME)), glm::vec3(0.0f, 1.0f, 0.0f));
			mainWindow.ctm.Scale(glm::vec3(1.0f, -1.0f, 1.0f));
			mainWindow.ctm.SetModel();
			RenderModel(fan, fan.scene->mRootNode);

			for (auto i = 0; i < NUM_OF_POINT_LIGHTS; ++i)
			{
				glDisable(GL_BLEND);
				mainWindow.ctm.LoadIdentity();
				mainWindow.ctm.Translate(pointLightLocations[i][0], 0.0f, pointLightLocations[i][1]); // y axis not needed
				mainWindow.ctm.Scale(glm::vec3(1.0f, -1.0f, 1.0f));
				mainWindow.ctm.SetModel();
				RenderModel(ceilingLamp, ceilingLamp.scene->mRootNode);
			}
		}

		glDisable(GL_STENCIL_TEST);
	}

	mainWindow.SetBlending();
	mainWindow.ctm.LoadIdentity();
	mainWindow.ctm.SetModel();
	RenderModel(ground, ground.scene->mRootNode);

	glDisable(GL_BLEND);
	mainWindow.ctm.LoadIdentity();
	mainWindow.ctm.SetModel();
	RenderModel(maze, maze.scene->mRootNode);

	// FPS computation and display
	frame++;
	time = glutGet(GLUT_ELAPSED_TIME);
	if (time - timebase > 1000)
	{
		frameRateText = "FPS: " + std::to_string(frame * 1000.0f / (time - timebase));
		timebase = time;
		frame = 0;
		const auto title = "SimpleGallery - " + frameRateText;
		glutSetWindowTitle(title.c_str());
	}

	if (mainWindow.showHelpInstructions)
	{
		renderHelpInstructions();
	}

	glutSwapBuffers();
}

bool oneTimeInit()
{
	shader.Setup("shaders/full");
	mainWindow.Init();
	mainWindow.SetShader(&shader);

	GLuint k = glGetUniformBlockIndex(shader(), "Matrices");
	glUniformBlockBinding(shader(), k, matricesUniLoc);
	glUniformBlockBinding(shader(), glGetUniformBlockIndex(shader(), "Material"), materialUniLoc);
	texUnit = glGetUniformLocation(shader(), "texUnit");

	maze.SetModelFile("models/maze/", "maze.obj");
	portrait.SetModelFile("models/screenshot-portrait/", "screenshot-portrait.obj");
	portraits.SetModelFile("models/portraits/", "portraits.obj");
	benches.SetModelFile("models/benches/", "benches.obj");
	ground.SetModelFile("models/floor/", "floor.obj");
	fan.SetModelFile("models/fan/", "fan.obj");
	pedestal.SetModelFile("models/pedestal/", "pedestal.obj");
	table.SetModelFile("models/table/", "table.obj");
	vases.SetModelFile("models/vases/", "vases.obj");
	ceilingLamp.SetModelFile("models/ceiling-lamp/", "ceiling-lamp.obj");
	star.SetModelFile("models/star/", "star.obj");
	pie.SetModelFile("models/pie/", "pie.obj");
	pentCrystal.SetModelFile("models/pent-crystal/", "pent-crystal.obj");
	pentPrism.SetModelFile("models/pent-prism/", "pent-prism.obj");

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

	glGenBuffers(1, &CTM::MatricesUniBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, CTM::MatricesUniBuffer);
	glBufferData(GL_UNIFORM_BUFFER, MatricesUniBufferSize, nullptr,GL_DYNAMIC_DRAW);
	glBindBufferRange(GL_UNIFORM_BUFFER, matricesUniLoc, CTM::MatricesUniBuffer, 0, MatricesUniBufferSize);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	return true;
}

int main(int argc, char* argv[])
{
	// GLUT init
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_ALPHA | GLUT_DEPTH | GLUT_STENCIL | GLUT_MULTISAMPLE);
	glutInitContextVersion(3, 3);
	//glutInitContextFlags(GLUT_DEBUG | GLUT_FORWARD_COMPATIBLE);
	glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE); // TODO Change to core profile
	glutInitWindowPosition((glutGet(GLUT_SCREEN_WIDTH) - WINDOW_WIDTH) / 2,
	                       (glutGet(GLUT_SCREEN_HEIGHT) - WINDOW_HEIGHT) / 2);
	glutInitWindowSize(WINDOW_WIDTH, WINDOW_HEIGHT);
	glutCreateWindow(WINDOW_TITLE.c_str());

	// GLUT callbacks
	glutDisplayFunc(displayCallback);
	glutReshapeFunc(reshapeCallback);
	glutKeyboardFunc(keyCallback);
	glutKeyboardUpFunc(keyUpCallback);
	glutSpecialFunc(specialCallback);
	glutSpecialUpFunc(specialUpCallback);
	glutIdleFunc(idleCallback);

	// GLEW init
	glewExperimental = GL_TRUE;
	glewInit();
	if (glewIsSupported("GL_VERSION_3_3"))
	{
		std::cout << "Ready for OpenGL 3.3" << std::endl;
	}
	else
	{
		std::cerr << "OpenGL 3.3 not supported" << std::endl;
		return false;
	}

	// devIL init
	ilInit();

	// App init
	if (!oneTimeInit())
	{
		return false;
	}

	std::cout << "Vender: " << glGetString(GL_VENDOR) << std::endl;
	std::cout << "Renderer: " << glGetString(GL_RENDERER) << std::endl;
	std::cout << "Version: " << glGetString(GL_VERSION) << std::endl;
	std::cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();

	// Cleanup
	glDeleteBuffers(1, &CTM::MatricesUniBuffer);

	return true;
}

void keyCallback(unsigned char key, int x, int y)
{
	mainWindow.HandleKey(key, x, y);
}

void keyUpCallback(unsigned char key, int x, int y)
{
	mainWindow.HandleKeyUp(key, x, y);
}

void specialCallback(int key, int x, int y)
{
	mainWindow.HandleSpecial(key, x, y);
}

void specialUpCallback(int key, int x, int y)
{
	mainWindow.HandleSpecialUp(key, x, y);
}

void idleCallback()
{
	auto currentFrame = static_cast<GLfloat>(glutGet(GLUT_ELAPSED_TIME) / 1000.0f);
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;
	mainWindow.HandleSmoothInput(deltaTime);
	glutPostRedisplay();
}
