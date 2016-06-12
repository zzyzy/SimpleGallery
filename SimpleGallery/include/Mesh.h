#pragma once

#include <GL/glew.h>

struct Mesh
{
    GLuint vao;
    GLuint texIndex;
    GLuint uniformBlockIndex;
    int numFaces;
};
