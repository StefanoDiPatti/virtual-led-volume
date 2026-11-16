/*
LineRenderer class
- it collects coloured segments in world coordinates during the frame (AddLine)
- it uploads them into a dynamic VBO and draws them with a single call (Draw), also more than once per frame
- it empties the list at the end of the frame (Clear)
*/

#pragma once

// Loader for GLM
#include <glm/glm.hpp>

#include <vector>
// offsetof, used for the vertex attribute layout
#include <cstddef>

// shader of the lines
#include <render/line_shader.h>
// class to manage OpenGL failures
#include <render/gl_check.h>

// vertex of a line: position in world coordinates (location 0) and colour (location 1)
struct LineVertex
{
    glm::vec3 position;
    glm::vec3 color;

    LineVertex(const glm::vec3& position, const glm::vec3& color)
    {
        this->position = position;
        this->color = color;
    }
};

/////////////////// LINE RENDERER class ///////////////////////
class LineRenderer
{
public:
    // constructor: creation of the shader and of the VBO, with room for maxVertices vertices
    LineRenderer(int maxVertices) : maxVertices(maxVertices), shader("shaders/line.vert", "shaders/line.frag")
    {
        this->SetupBuffers();
    }

    // the instance owns OpenGL resources: copies are not allowed
    LineRenderer(const LineRenderer&) = delete;
    LineRenderer& operator=(const LineRenderer&) = delete;

    // destructor: we delete the buffers
    ~LineRenderer()
    {
        GL_CHECK(glDeleteVertexArrays(1, &this->VAO));
        GL_CHECK(glDeleteBuffers(1, &this->VBO));
    }

    // we add a segment from a to b (two vertices with the same colour)
    void AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color)
    {
        // the VBO is full: the segment is discarded
        if((int)vertices.size() + 2 > maxVertices)
            return;

        vertices.emplace_back(a, color);
        vertices.emplace_back(b, color);
    }

    // we upload the segments of the frame into the VBO and we draw them
    void Draw(const glm::mat4& view, const glm::mat4& projection)
    {
        if (vertices.empty())
            return;

        shader.Begin(view, projection);
        GL_CHECK(glBindVertexArray(this->VAO));

        // the VBO is already allocated: we only copy the vertices of this frame, from the beginning
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
        GL_CHECK(glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(LineVertex), vertices.data()));

        // GL_LINES: every pair of vertices is a separate segment
        GL_CHECK(glDrawArrays(GL_LINES, 0, (GLsizei)vertices.size()));

        GL_CHECK(glBindVertexArray(0));
    }

    // we empty the list at the end of the frame (the VBO stays allocated)
    void Clear()
    {
        vertices.clear();
    }

private:
    // capacity of the VBO (number of vertices)
    int maxVertices = 0;
    // buffer objects\arrays of the lines
    GLuint VAO = 0, VBO = 0;

    // segments of the current frame (two vertices for each segment)
    std::vector<LineVertex> vertices;
    LineShader shader;

    // creation of the VAO and of an empty VBO: positions (location 0) and colours (location 1), interleaved
    void SetupBuffers()
    {
        GL_CHECK(glGenVertexArrays(1, &this->VAO));
        GL_CHECK(glGenBuffers(1, &this->VBO));

        // VAO is made "active"
        GL_CHECK(glBindVertexArray(this->VAO));
        // we allocate the VBO without data (nullptr): it is filled at every Draw
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, this->VBO));
        GL_CHECK(glBufferData(GL_ARRAY_BUFFER, maxVertices * sizeof(LineVertex), nullptr,  GL_DYNAMIC_DRAW));

        // stride: size of one vertex; the colour starts after the position
        GL_CHECK(glEnableVertexAttribArray(0));
        GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (GLvoid*)0));

        GL_CHECK(glEnableVertexAttribArray(1));
        GL_CHECK(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (GLvoid*)(offsetof(LineVertex, color))));

        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
        GL_CHECK(glBindVertexArray(0));
    }
};
