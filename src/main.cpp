/*
OpenGL coordinate system (right-handed)
positive X axis points right
positive Y axis points up
positive Z axis points "outside" the screen


                              Y
                              |
                              |
                              |________X
                             /
                            /
                           /
                          Z
*/

// Std. Includes
#include <string>

// Loader for OpenGL extensions
#include <glad/glad.h>

// class to manage OpenGL failures
#include "gl_check.h"

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// class to manage shaders 
#include <utils/shader.h>

// class to include models
#include <utils/model.h>

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// dimensions of application's window
GLuint screenWidth = 1920, screenHeight = 1080;

// setup of Shader Programs used in the application
void SetupShader(Shader& shader, const char* vertexPath, const char* fragmentPath);
// delete Shader Programs when application ends
void DeleteShader(Shader& shader);

// ------- Materials and Lighting -------
// Uniforms to pass to shaders
// position of a pointlight
glm::vec3 lightPos = glm::vec3(5.0f, 10.0f, 10.0f);
//GLfloat lightColor[] = {1.0f,1.0f,1.0f};

// diffusive, specular and ambient components
GLfloat yodaMaterial[] = {1.0f,0.0f,0.0f};
GLfloat specularColor[] = {1.0f,1.0f,1.0f};
GLfloat ambientColor[] = {0.1f,0.1f,0.1f};
// weights for the diffusive, specular and ambient components
GLfloat Kd = 0.5f;
GLfloat Ks = 0.4f;
GLfloat Ka = 0.1f;
// shininess coefficient for Blinn-Phong shaders
GLfloat shininess = 25.0f;

GLfloat planeMaterial[] = {0.0f,0.5f,0.0f};
// variables used to store uniform location inside shaders
GLint pointLightLocation, matDiffuseLocation, matAmbientLocation, matSpecularLocation;  
GLint kdLocation, kaLocation, ksLocation, shineLocation;

// callback function for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void fb_size_callback(GLFWwindow* window, int width, int height);

/////////////////// MAIN function ///////////////////////
int main()
{
    // initialization of OpenGL context using GLFW
    int glfwStatus = glfwInit();
    if(glfwStatus == GLFW_FALSE)
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL 4.1 Core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // we set if the window is resizable
    glfwWindowHint(GLFW_RESIZABLE, GL_TRUE);

    // creation of the application's window
    GLFWwindow* window = glfwCreateWindow(screenWidth, screenHeight, "virtual-led-volume", nullptr, nullptr);
    if (!window)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // relate window and the callbacks
    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, fb_size_callback);

    // GLAD tries to load the context set by GLFW
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress))
    {
        std::cout << "Failed to initialize OpenGL context" << std::endl;
        glfwTerminate();
        return -1;
    }

    std::cout << "OpenGL version: " << (const char*)glGetString(GL_VERSION) << std::endl;
    std::cout << "Renderer: " << (const char*)glGetString(GL_RENDERER) << std::endl;
    std::cout << "GLSL version: " << (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

    // viewport dimensions
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    GL_CHECK(glViewport(0, 0, width, height));

    // enable Z test
    GL_CHECK(glEnable(GL_DEPTH_TEST));

    // creation of the Shader Programs
    Shader shader, wallShader;
    SetupShader(shader, "shaders/scene.vert", "shaders/scene.frag");
    SetupShader(wallShader, "shaders/wall.vert", "shaders/wall.frag");

    // ------- Models -------
    Model planeModel("assets/models/plane.obj");
    Model yodaModel("assets/models/babyyoda.obj");

    // we set projection and view matrices
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
    // View matrix (=camera): position, view direction, camera "up" vector
    glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 1.7f, 6.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    // Model and Normal transformation matrices for the objects in the scene
    glm::mat4 planeModelMatrix = glm::mat4(1.0f);
    glm::mat3 planeNormalMatrix = glm::mat3(1.0f);
    glm::mat4 yodaModelMatrix = glm::mat4(1.0f);
    glm::mat3 yodaNormalMatrix = glm::mat3(1.0f);

    // ------- Lighting -------
    // we determine the position in the Shader Program of the uniform variables
    pointLightLocation = glCheckUniformLocation(shader.Program, "pointLightPosition");
    matDiffuseLocation = glCheckUniformLocation(shader.Program, "diffuseColor");
    matAmbientLocation = glCheckUniformLocation(shader.Program, "ambientColor");
    matSpecularLocation = glCheckUniformLocation(shader.Program, "specularColor");
    kaLocation = glCheckUniformLocation(shader.Program, "Ka");
    kdLocation = glCheckUniformLocation(shader.Program, "Kd");
    ksLocation = glCheckUniformLocation(shader.Program, "Ks");
    shineLocation = glCheckUniformLocation(shader.Program, "shininess");

    // ------- LED Wall --------
    glm::mat4 wallModelMatrix = glm::mat4(1.0f);

    // the LED wall quad: 4 corners in world coordinates (pd = pb + pc - pa)
    GLfloat vertices[] =
    {
        -3.0f,  0.0f, -3.0f, 0.0f, 0.0f,         // pa: lower left
        3.0f, 0.0f, -3.0f,  1.0f, 0.0f,          // pb: lower-right
        -3.0f,  3.0f, -3.0f, 0.0f, 1.0f,    // pc: upper-left
        3.0f,  3.0f, -3.0f,  1.0f, 1.0f     // pd: upper right
    };
    GLuint indices[] =
    {
        0, 1, 3,  // First Triangle (pa, pb, pd)
        0, 3, 2   // Second Triangle (pa, pd, pc)
    };

    // ------- Buffer objects\arrays -------
    GLuint wallVBO, wallVAO, wallEBO;
    GL_CHECK(glGenVertexArrays(1, &wallVAO));
    GL_CHECK(glGenBuffers(1, &wallVBO));
    GL_CHECK(glGenBuffers(1, &wallEBO));
    
    // wallVAO is made "active"    
    GL_CHECK(glBindVertexArray(wallVAO));
    // we copy data in the wallVBO - we must set the data dimension, and the pointer to the structure cointaining the data
    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, wallVBO));
    GL_CHECK(glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW));
    // we copy data in the wallEBO - we must set the data dimension, and the pointer to the structure cointaining the data
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, wallEBO));
    GL_CHECK(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW));

    // vertex positions (location 0) and UVs (location 2)
    GL_CHECK(glEnableVertexAttribArray(0));
    GL_CHECK(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0));

    GL_CHECK(glEnableVertexAttribArray(2));
    GL_CHECK(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat))));

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, 0));
    GL_CHECK(glBindVertexArray(0)); // Unbind wallVAO

    /////////////////// RENDERING LOOP ///////////////////////
    // this code is executed at each frame
    while(!glfwWindowShouldClose(window))
    {
        // Check if an I/O event is happening
        glfwPollEvents();

        // "clear" the frame and z-buffer
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        // usage of Shader Program as part of the current rendering process
        shader.Use();

        // we assign the value to the uniform variables
        GL_CHECK(glUniform3fv(pointLightLocation, 1, glm::value_ptr(lightPos)));
        GL_CHECK(glUniform3fv(matAmbientLocation, 1, ambientColor));
        GL_CHECK(glUniform3fv(matSpecularLocation, 1, specularColor));
        GL_CHECK(glUniform1f(kaLocation, Ka));
        GL_CHECK(glUniform1f(kdLocation, Kd));
        GL_CHECK(glUniform1f(ksLocation, Ks));
        GL_CHECK(glUniform1f(shineLocation, shininess));

        // we pass projection and view matrices to the Shader Program
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection)));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view)));

        // -----------PLANE-----------
        // reset to identity at each frame
        planeModelMatrix = glm::mat4(1.0f);
        planeNormalMatrix = glm::mat3(1.0f);
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        planeNormalMatrix = glm::inverseTranspose(glm::mat3(view * planeModelMatrix));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(planeModelMatrix)));
        GL_CHECK(glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normals"), 1, GL_FALSE, glm::value_ptr(planeNormalMatrix)));

        GL_CHECK(glUniform3fv(matDiffuseLocation, 1, planeMaterial));

        // we render the plane
        planeModel.Draw();

        // -----------YODA-----------
        // reset to identity at each frame
        yodaModelMatrix = glm::mat4(1.0f);
        yodaNormalMatrix = glm::mat3(1.0f);
        yodaModelMatrix = glm::scale(yodaModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));	// too big, scale it down
        // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
        yodaNormalMatrix = glm::inverseTranspose(glm::mat3(view * yodaModelMatrix));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(yodaModelMatrix)));
        GL_CHECK(glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normals"), 1, GL_FALSE, glm::value_ptr(yodaNormalMatrix)));

        GL_CHECK(glUniform3fv(matDiffuseLocation, 1, yodaMaterial));

        // we render yoda
        yodaModel.Draw();

        // -----------LED WALL-----------

        wallShader.Use();

        // we pass projection and view matrices to the Wall Shader
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(wallShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wallModelMatrix)));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(wallShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view)));
        GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(wallShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection)));

        GL_CHECK(glBindVertexArray(wallVAO));
        GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
        GL_CHECK(glBindVertexArray(0));

        // double buffering: swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    DeleteShader(shader);
    DeleteShader(wallShader);

    glDeleteVertexArrays(1, &wallVAO);
    glDeleteBuffers(1, &wallVBO);
    glDeleteBuffers(1, &wallEBO);

    // close and delete context
    glfwTerminate();
    return 0;
}

/////////////////// SHADERS MANAGEMENT ///////////////////////
void SetupShader(Shader& shader, const char* vertexPath, const char* fragmentPath)
{
    shader = Shader(vertexPath, fragmentPath);
}

void DeleteShader(Shader& shader)
{
    shader.Delete();
}

/////////////////// CALLBACK FOR KEYBOARD EVENTS ///////////////////////
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
}

void fb_size_callback(GLFWwindow* window, int width, int height)
{
    GL_CHECK(glViewport(0, 0, width, height));
}
