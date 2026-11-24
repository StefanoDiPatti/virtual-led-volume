/*
LineRenderer class
- it collects coloured segments in world coordinates during the frame (AddLine)
- it uploads them into a dynamic VBO and draws them with a single call (Draw), also more than once per frame
- it empties the list at the end of the frame (Clear)
*/

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <array>
#include <cstddef>
#include <render/line_shader.h>
#include <core/config.h>
#include <render/gl_check.h>

// groups of debug lines drawn in the observer views (switched on and off from the GUI)
struct LineToggles
{
    bool worldAxes = true, wallFrame = true, cineFrustum = true, cameraGizmo = true, offAxisLines = true;
};

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
    LineRenderer(int maxVertices) : maxVertices(maxVertices), shader(LINE_VERT_PATH, LINE_FRAG_PATH)
    {
        SetupBuffers();
    }

    LineRenderer(const LineRenderer&) = delete;
    LineRenderer& operator=(const LineRenderer&) = delete;

    ~LineRenderer()
    {
        GL_CHECK(glDeleteVertexArrays(1, &VAO));
        GL_CHECK(glDeleteBuffers(1, &VBO));
    }

    void AddLine(const glm::vec3& a, const glm::vec3& b, const glm::vec3& color)
    {
        // the VBO is full: the segment is discarded
        if((int)vertices.size() + 2 > maxVertices)
            return;

        vertices.emplace_back(a, color);
        vertices.emplace_back(b, color);
    }

    // we add the 12 edges of a box given its 8 corners (0-3 first face, 4-7 opposite face, same order)
    void AddBox(const std::array<glm::vec3, 8>& corners, const glm::vec3& color)
    {
        for(int i = 0; i < 4; i++)
        {
            // next corner of the face (the last one goes back to the first)
            int j = (i + 1) % 4;
            AddLine(corners[i], corners[j], color);
            AddLine(corners[i + 4], corners[j + 4], color);
            AddLine(corners[i], corners[i + 4], color);
        }
    }

    // we add three axes from the origin (red, green, blue), each one long length (the axes must be unit vectors)
    void AddAxes(const glm::vec3& origin, const glm::vec3& xAxis, const glm::vec3& yAxis, const glm::vec3& zAxis, float length)
    {
        AddLine(origin, origin + xAxis * length, config::AXIS_X_COLOR);
        AddLine(origin, origin + yAxis * length, config::AXIS_Y_COLOR);
        AddLine(origin, origin + zAxis * length, config::AXIS_Z_COLOR);
    }

    // we upload the segments of the frame into the VBO and we draw them
    void Draw(const glm::mat4& view, const glm::mat4& projection)
    {
        if (vertices.empty())
            return;

        shader.Begin(view, projection);
        GL_CHECK(glBindVertexArray(VAO));

        // the VBO is already allocated: we only copy the vertices of this frame, from the beginning
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
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
    int maxVertices = 0;
    GLuint VAO = 0, VBO = 0;

    // segments of the current frame (two vertices for each segment)
    std::vector<LineVertex> vertices;
    LineShader shader;

    // creation of the VAO and of an empty VBO: positions (location 0) and colours (location 1), interleaved
    void SetupBuffers()
    {
        GL_CHECK(glGenVertexArrays(1, &VAO));
        GL_CHECK(glGenBuffers(1, &VBO));

        GL_CHECK(glBindVertexArray(VAO));
        // we allocate the VBO without data (nullptr): it is filled at every Draw
        GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, VBO));
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
