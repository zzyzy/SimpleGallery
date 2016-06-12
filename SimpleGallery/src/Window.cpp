#include "Window.h"

#include <iostream>
#include <fstream>

#include <GL/freeglut.h>
#include <IL/il.h>
#include <IL/ilut.h>

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::ifstream;
using std::ofstream;
using std::to_string;

Window::Window(const int& w, const int& h) : width(w), height(h)
{
    
}

Window::~Window()
{
    glDeleteBuffers(1, &blackMatId);
    glDeleteBuffers(1, &whiteMatId);
    glDeleteBuffers(1, &yellowMatId);
    glDeleteTextures(1, &screenshotTexId);
}

void Window::Init()
{
    cameraStartPos = glm::vec3(0.0f, 0.0f, 3.0f);
    camera.Setup(cameraStartPos);

    drawingMode = DrawingMode::SOLID;

    wireframeMode = WireframeMode::BLACK_WHITE;
    float blackColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    memcpy(blackMat.ambient, blackColor, sizeof(blackColor));
    memcpy(blackMat.diffuse, blackColor, sizeof(blackColor));
    memcpy(blackMat.specular, blackColor, sizeof(blackColor));
    memcpy(blackMat.emissive, blackColor, sizeof(blackColor));
    blackMat.shininess = 0.6f * 128;
    blackMat.texCount = 0;
    glGenBuffers(1, &blackMatId);
    glBindBuffer(GL_UNIFORM_BUFFER, blackMatId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(blackMat), static_cast<void *>(&blackMat), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    float whiteColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    memcpy(whiteMat.ambient, whiteColor, sizeof(whiteColor));
    memcpy(whiteMat.diffuse, whiteColor, sizeof(whiteColor));
    memcpy(whiteMat.specular, whiteColor, sizeof(whiteColor));
    memcpy(whiteMat.emissive, whiteColor, sizeof(whiteColor));
    whiteMat.shininess = 0.6f * 128;
    whiteMat.texCount = 0;
    glGenBuffers(1, &whiteMatId);
    glBindBuffer(GL_UNIFORM_BUFFER, whiteMatId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(whiteMat), static_cast<void *>(&whiteMat), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    float yellowColor[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
    memcpy(yellowMat.ambient, yellowColor, sizeof(yellowColor));
    memcpy(yellowMat.diffuse, yellowColor, sizeof(yellowColor));
    memcpy(yellowMat.specular, yellowColor, sizeof(yellowColor));
    memcpy(yellowMat.emissive, yellowColor, sizeof(yellowColor));
    yellowMat.shininess = 0.6f * 128;
    yellowMat.texCount = 0;
    glGenBuffers(1, &yellowMatId);
    glBindBuffer(GL_UNIFORM_BUFFER, yellowMatId);
    glBufferData(GL_UNIFORM_BUFFER, sizeof(yellowMat), static_cast<void *>(&yellowMat), GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);

    currentMatId = blackMatId;

    solidMode = SolidMode::FULL;
    setDrawingMode = false;

    antiAliasing = false;
    setAntiAliasing = false;

    glGenTextures(1, &screenshotTexId);
    ifstream infile("screenshots/.screenshots", ifstream::binary);
    if (infile.is_open())
    {
        string filename = "screenshots/screenshot-";
        int count;

        infile.seekg(0, infile.end);
        auto end = infile.tellg();
        infile.clear();
        char buffer[sizeof count];
        infile.seekg(end - std::streampos(sizeof count), infile.beg);
        infile.read(buffer, sizeof count);
        count = atoi(buffer);
        cout << "Last screenshot number: \"" << count << "\"" << endl;

        filename += to_string(count) + ".png";
        auto imageID = ilGenImage();
        ilBindImage(imageID);
        ilEnable(IL_ORIGIN_SET);
        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
        ilLoadImage(filename.c_str());
        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);

        glBindTexture(GL_TEXTURE_2D, screenshotTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
            ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
            ilGetData());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        ilDeleteImage(imageID);
        ilBindImage(0);
    }
    infile.close();

    showHelpInstructions = true;
    timeOfDay = true;
    setTimeOfDay = false;
    lighting = true;
    flashLightOn = false;

    blending = false;
    setBlending = false;

    textured = true;

    for (auto i = 0; i < 9; ++i)
    {
        lights[i] = true;
    }

    for (auto i = 0; i < 2; ++i)
    {
        spotLights[i] = true;
    }

    intensity = 1.0f;
    flashLightBaseColor = glm::vec3(0.8f, 0.8f, 0.8f);
    flashLightDiffuse = flashLightBaseColor;
    flashLightColor[0] = glm::vec3(1.0f, 0.0f, 0.0f);
    flashLightColor[1] = glm::vec3(0.0f, 1.0f, 0.0f);
    flashLightColor[2] = glm::vec3(0.0f, 0.0f, 1.0f);
}

void Window::Display()
{
}

void Window::SetDrawingMode()
{
    //if (!setDrawingMode) return;

    if (drawingMode == DrawingMode::WIREFRAME)
    {
        lighting = false;
        textured = false;
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        switch (wireframeMode)
        {
        case WireframeMode::BLACK_WHITE:
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            currentMatId = whiteMatId;
            break;
        case WireframeMode::WHITE_BLACK:
            glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
            currentMatId = blackMatId;
            break;
        case WireframeMode::BLUE_YELLOW:
            glClearColor(0.0f, 0.0f, 1.0f, 1.0f);
            currentMatId = yellowMatId;
            break;
        }
    } 
    else
    {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        switch (solidMode)
        {
        case SolidMode::BASIC:
            lighting = false;
            textured = false;
            break;
        case SolidMode::LIGHTINGONLY:
            lighting = true;
            textured = false;
            break;
        case SolidMode::TEXTUREDONLY:
            lighting = false;
            textured = true;
            break;
        case SolidMode::FULL:
            lighting = true;
            textured = true;
            break;
        }
    }

    //setDrawingMode = false;
}

void Window::SetAntiAliasing() const
{
    //if (!setAntiAliasing) return;

    if (antiAliasing)
    {
        glEnable(GL_MULTISAMPLE);
    } 
    else
    {
        glDisable(GL_MULTISAMPLE);
    }

    //setAntiAliasing = false;
}

void Window::SetBlending() const
{
    //if (!setBlending) return;

    if (blending)
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else
    {
        glDisable(GL_BLEND);
    }

    //setBlending = false;
}

void Window::SetShader(Shader* shader)
{
    _shader = shader;
}

void Window::SetTexture() const
{
    glUniform1i(glGetUniformLocation((*_shader)(), "isTextured"), textured);
}

void Window::SetTimeOfDay() const
{
    //if (!setTimeOfDay) return;

    if (timeOfDay)
    {
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    }
    else
    {
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    }

    //setTimeOfDay = false;
}

void Window::SetViewMatrix(const Shader& shader) const
{
    ctm.SetView(camera.GetViewMatrix());
    glUniform3f(glGetUniformLocation(shader(), "viewPos"), camera.Position.x, camera.Position.y, camera.Position.z);
}

string Window::GetDisplayStateString()
{
    string displayState = "Display state: ";
    if (drawingMode == DrawingMode::WIREFRAME)
    {
        switch (wireframeMode)
        {
        case WireframeMode::BLACK_WHITE:
            return displayState + "wireframe mode with black background and white lines";
        case WireframeMode::WHITE_BLACK:
            return displayState + "wireframe mode with white background and black lines";
        case WireframeMode::BLUE_YELLOW:
            return displayState + "wireframe mode with blue background and yellow lines";
        }
    }
    else
    {
        switch (solidMode)
        {
        case SolidMode::BASIC:
            return displayState + "solid mode with colors";
        case SolidMode::LIGHTINGONLY:
            return displayState + "solid mode with lighting only";
        case SolidMode::TEXTUREDONLY:
            return displayState + "solid mode with textures only";
        case SolidMode::FULL:
            return displayState + "solid mode with smooth shading";
        }
    }
}



void Window::HandleKey(unsigned char key, int x, int y)
{
    if (key == GLUT_KEY_ESCAPE)
        return;

    // Screenshot controls
    if (key == 'p') // Take screenshot
    {
        string filename = "screenshots/screenshot-";
        auto count = 0;

        // Find the last screenshot number
        ifstream infile("screenshots/.screenshots", ifstream::binary);
        if (infile.is_open())
        {
            infile.seekg(0, infile.end);
            auto end = infile.tellg();
            infile.clear();
            char buffer[sizeof count];
            infile.seekg(end - std::streampos(sizeof count), infile.beg);
            infile.read(buffer, sizeof count);
            count = atoi(buffer);
            cout << "Last screenshot number: \"" << count << "\"" << endl;
            count++;
        }
        infile.close();

        filename += to_string(count) + ".png";

        auto imageID = ilGenImage();
        ilBindImage(imageID);
        ilutGLScreen();
        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
        //ilEnable(IL_FILE_OVERWRITE); // don't allow overwriting as per requirement specification
        ilSave(IL_PNG, filename.c_str());
        cout << "Screenshot taken and saved as \"" << filename << "\"" << endl;

        // Save the last screenshot number
        ofstream outfile("screenshots/.screenshots", ofstream::binary | ofstream::app);
        if (outfile.is_open())
        {
            outfile.write(to_string(count).c_str(), sizeof count);
        }
        outfile.close();

        glBindTexture(GL_TEXTURE_2D, screenshotTexId);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ilGetInteger(IL_IMAGE_WIDTH),
            ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGBA, GL_UNSIGNED_BYTE,
            ilGetData());
        glGenerateMipmap(GL_TEXTURE_2D);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glBindTexture(GL_TEXTURE_2D, 0);

        ilDeleteImage(imageID);

        return;
    }

    // Wireframe mode configurations
    if (key == 'z') // Black background with white lines
    {
        drawingMode = DrawingMode::WIREFRAME;
        wireframeMode = WireframeMode::BLACK_WHITE;
        cout << "Drawing in wireframe mode using black background and white lines" << endl;
        setDrawingMode = true;
        return;
    }
    if (key == 'x') // White background with black lines
    {
        drawingMode = DrawingMode::WIREFRAME;
        wireframeMode = WireframeMode::WHITE_BLACK;
        cout << "Drawing in wireframe mode using white background and black lines" << endl;
        setDrawingMode = true;
        return;
    }
    if (key == 'c') // Blue background with yellow lines
    {
        drawingMode = DrawingMode::WIREFRAME;
        wireframeMode = WireframeMode::BLUE_YELLOW;
        cout << "Drawing in wireframe mode using blue background and yellow lines" << endl;
        setDrawingMode = true;
        return;
    }

    // Solid mode configurations
    if (key == 'v') // Colored differently but without lighting or textures
    {
        drawingMode = DrawingMode::SOLID;
        solidMode = SolidMode::BASIC;
        cout << "Drawing in solid mode using basic colors only" << endl;
        setDrawingMode = true;
        return;
    }
    if (key == 'b') // Shaded using lighting only
    {
        drawingMode = DrawingMode::SOLID;
        solidMode = SolidMode::LIGHTINGONLY;
        cout << "Drawing in solid mode using basic lighting only" << endl;
        setDrawingMode = true;
        return;
    }
    if (key == 'n') // Textured without lighting
    {
        drawingMode = DrawingMode::SOLID;
        solidMode = SolidMode::TEXTUREDONLY;
        cout << "Drawing in solid mode using textures only" << endl;
        setDrawingMode = true;
        return;
    }
    if (key == 'm') // Shaded using lighting and texture
    {
        drawingMode = DrawingMode::SOLID;
        solidMode = SolidMode::FULL;
        cout << "Drawing in solid mode using full smooth lighting and textures" << endl;
        setDrawingMode = true;
        return;
    }

    // Light controls
    if (key - '0' >= 1 && key - '0' <= 9) // On/Off light no.1
    {
        lights[key - '0' - 1] = !lights[key - '0' - 1];
        cout << "Light " << key - '0' << " turned " << (lights[key - '0' - 1] ? "on" : "off") << endl;
        return;
    }
    if (key == ';')
    {
        spotLights[0] = !spotLights[0];
        cout << "Spot Light 1 turned " << (spotLights[0] ? "on" : "off") << endl;
        return;
    }
    if (key == '\'')
    {
        spotLights[1] = !spotLights[1];
        cout << "Spot Light 2 turned " << (spotLights[1] ? "on" : "off") << endl;
        return;
    }

    // Translucent surface control
    // Blending control
    if (key == 't') // Toggle between opaque and translucent
    {
        blending = !blending;
        cout << "Blending turned " << (blending ? "on" : "off") << endl;
        setBlending = true;
        return;
    }

    // Flashlight controls
    if (key == 'f') // On/Off
    {
        flashLightOn = !flashLightOn;
        cout << "Flash light turned " << (flashLightOn ? "on" : "off") << endl;
        return;
    }
    if (key == 'j') // Increase intensity
    {
        intensity += 0.20f;
        /*if (intensity >= 1.00f)
        {
            intensity = 1.00f;
        }*/
        cout << "Flash light intensity: " << intensity << endl;
        return;
    }
    if (key == 'k') // Decrease intensity
    {
        intensity -= 0.20f;
        if (intensity <= 0.00f)
        {
            intensity = 0.00f;
        }
        cout << "Flash light intensity: " << intensity << endl;
        return;
    }
    if (key == 'l') // Cycle color between red, green and blue
    {
        std::cout << "Flash light color: ";
        if (flashLightDiffuse == flashLightBaseColor)
        {
            flashLightDiffuse = flashLightColor[0];
            cout << "Red" << endl;
        } else if (flashLightDiffuse == flashLightColor[0])
        {
            flashLightDiffuse = flashLightColor[1];
            cout << "Green" << endl;
        }
        else if (flashLightDiffuse == flashLightColor[1])
        {
            flashLightDiffuse = flashLightColor[2];
            cout << "Blue" << endl;
        }
        else if (flashLightDiffuse == flashLightColor[2])
        {
            flashLightDiffuse = flashLightBaseColor;
            cout << "White" << endl;
        }
        return;
    }

    // Anti aliasing control
    if (key == 'o') // On/Off
    {
        antiAliasing = !antiAliasing;
        cout << "Anti aliasing turned " << (antiAliasing ? "on" : "off") << endl;
        setAntiAliasing = true;
        return;
    }

    // Show/Hide help instructions
    if (key == 'h')
    {
        showHelpInstructions = !showHelpInstructions;
        return;
    }

    // Time of day control
    if (key == 'i')
    {
        timeOfDay = !timeOfDay;
        cout << "Time of day: " << (timeOfDay ? "Day" : "Night") << " time" << endl;
        setTimeOfDay = true;
        return;
    }

    // Other keys
    if (key >= 0 && key < 1024)
    {
        keys[key] = true;
    }
}

void Window::HandleKeyUp(unsigned char key, int x, int y)
{
    if (key == GLUT_KEY_ESCAPE)
        glutLeaveMainLoop();
    if (key >= 0 && key < 1024)
    {
        keys[key] = false;
    }
}

void Window::HandleSpecial(int key, int x, int y)
{
    if (key == GLUT_KEY_PAGE_UP)
        keys[GLUT_KEY_PAGE_UP_CUSTOM] = true;
    else if (key == GLUT_KEY_PAGE_DOWN)
        keys[GLUT_KEY_PAGE_DOWN_CUSTOM] = true;

    if (key == GLUT_KEY_HOME)
        keys[GLUT_KEY_HOME_CUSTOM] = true;
    else if (key == GLUT_KEY_END)
        keys[GLUT_KEY_END_CUSTOM] = true;

    if (key == GLUT_KEY_UP)
        keys[GLUT_KEY_UP_CUSTOM] = true;
    else if (key == GLUT_KEY_DOWN)
        keys[GLUT_KEY_DOWN_CUSTOM] = true;
    else if (key == GLUT_KEY_LEFT)
        keys[GLUT_KEY_LEFT_CUSTOM] = true;
    else if (key == GLUT_KEY_RIGHT)
        keys[GLUT_KEY_RIGHT_CUSTOM] = true;
}

void Window::HandleSpecialUp(int key, int x, int y)
{
    if (key == GLUT_KEY_PAGE_UP)
        keys[GLUT_KEY_PAGE_UP_CUSTOM] = false;
    else if (key == GLUT_KEY_PAGE_DOWN)
        keys[GLUT_KEY_PAGE_DOWN_CUSTOM] = false;

    if (key == GLUT_KEY_HOME)
        keys[GLUT_KEY_HOME_CUSTOM] = false;
    else if (key == GLUT_KEY_END)
        keys[GLUT_KEY_END_CUSTOM] = false;

    if (key == GLUT_KEY_UP)
        keys[GLUT_KEY_UP_CUSTOM] = false;
    else if (key == GLUT_KEY_DOWN)
        keys[GLUT_KEY_DOWN_CUSTOM] = false;
    else if (key == GLUT_KEY_LEFT)
        keys[GLUT_KEY_LEFT_CUSTOM] = false;
    else if (key == GLUT_KEY_RIGHT)
        keys[GLUT_KEY_RIGHT_CUSTOM] = false;
}

void Window::HandleSmoothInput(const GLfloat& deltaTime)
{
    // Camera controls
    if (keys['w'])
    {
        camera.ProcessKeyboard(Camera_Movement::FORWARD, deltaTime);
    }

    if (keys['s'])
    {
        camera.ProcessKeyboard(Camera_Movement::BACKWARD, deltaTime);
    }

    if (keys['a'])
    {
        camera.ProcessKeyboard(Camera_Movement::LEFT, deltaTime);
    }

    if (keys['d'])
    {
        camera.ProcessKeyboard(Camera_Movement::RIGHT, deltaTime);
    }

    if (keys[GLUT_KEY_PAGE_UP_CUSTOM])
    {
        camera.ProcessKeyboard(Camera_Movement::UP, deltaTime);
    }

    if (keys[GLUT_KEY_PAGE_DOWN_CUSTOM])
    {
        camera.ProcessKeyboard(Camera_Movement::DOWN, deltaTime);
    }

    if (keys[GLUT_KEY_HOME_CUSTOM])
    {
        camera.SetZoom(50.0f * deltaTime);
    }

    if (keys[GLUT_KEY_END_CUSTOM])
    {
        camera.SetZoom(-50.0f * deltaTime);
    }

    if (keys[GLUT_KEY_UP_CUSTOM])
    {
        camera.SetLookAt(0.0f, 75.0f * deltaTime, 0.0f);
    }

    if (keys[GLUT_KEY_DOWN_CUSTOM])
    {
        camera.SetLookAt(0.0f, -75.0f * deltaTime, 0.0f);
    }

    if (keys[GLUT_KEY_LEFT_CUSTOM])
    {
        camera.SetLookAt(-75.0f * deltaTime, 0.0f, 0.0f);
    }

    if (keys[GLUT_KEY_RIGHT_CUSTOM])
    {
        camera.SetLookAt(75.0f * deltaTime, 0.0f, 0.0f);
    }

    if (keys['q'])
    {
        camera.SetLookAt(0.0f, 0.0f, 50.0f * deltaTime);
    }

    if (keys['e'])
    {
        camera.SetLookAt(0.0f, 0.0f, -50.0f * deltaTime);
    }

    if (keys['0'])
    {
        camera.ResetToPosition(cameraStartPos);
    }
}
