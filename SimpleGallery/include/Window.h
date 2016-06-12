#pragma once
#ifndef WINDOW_H_INCLUDED
#define WINDOW_H_INCLUDED

#include <GL/glew.h>

#include "CTM.h"
#include "Camera.h"
#include "Model.h"
#include "Shader.h"

#define GLUT_KEY_ESCAPE 27
#define GLUT_KEY_PAGE_UP_CUSTOM 1000
#define GLUT_KEY_PAGE_DOWN_CUSTOM 1001
#define GLUT_KEY_HOME_CUSTOM 1002
#define GLUT_KEY_END_CUSTOM 1003
#define GLUT_KEY_UP_CUSTOM 1004
#define GLUT_KEY_DOWN_CUSTOM 1005
#define GLUT_KEY_LEFT_CUSTOM 1006
#define GLUT_KEY_RIGHT_CUSTOM 1007

enum class DrawingMode
{
    WIREFRAME,
    SOLID
};

enum class WireframeMode
{
    BLACK_WHITE,
    WHITE_BLACK,
    BLUE_YELLOW
};

enum class SolidMode
{
    BASIC,
    LIGHTINGONLY,
    TEXTUREDONLY,
    FULL
};

struct Window
{
    Window(const int& w, const int& h);
    ~Window();

    int width, height;

    // Current transformation matrix stack
    CTM ctm;

    // User input map
    bool keys[1024] = {false};

    // Camera
    glm::vec3 cameraStartPos;
    Camera camera;

    // Drawing mode
    DrawingMode drawingMode;

    WireframeMode wireframeMode;
    Material blackMat, whiteMat, yellowMat;
    GLuint blackMatId, whiteMatId, yellowMatId, currentMatId;

    SolidMode solidMode;
    bool setDrawingMode;

    bool antiAliasing;
    bool setAntiAliasing;

    GLuint screenshotTexId;

    bool showHelpInstructions;
    bool timeOfDay; // true = day, false = night
    bool setTimeOfDay;
    bool lighting;

    bool flashLightOn;
    GLfloat intensity;
    glm::vec3 flashLightDiffuse;
    glm::vec3 flashLightBaseColor;
    glm::vec3 flashLightColor[3];

    bool blending;
    bool setBlending;

    bool textured;

    Shader* _shader;

    bool lights[9];
    bool spotLights[2];

    // General methods
    void Init();
    void Display();

    // Configuration methods
    void SetShader(Shader* shader);
    void SetDrawingMode();
    void SetAntiAliasing() const;
    void SetBlending() const;
    void SetTexture() const;
    void SetTimeOfDay() const;
    void SetViewMatrix(const Shader& shader) const;

    std::string GetDisplayStateString();

    // User input callbacks
    void HandleKey(unsigned char key, int x, int y);
    void HandleKeyUp(unsigned char key, int x, int y);
    void HandleSpecial(int key, int x, int y);
    void HandleSpecialUp(int key, int x, int y);
    void HandleSmoothInput(const GLfloat& deltaTime);
};

#endif
