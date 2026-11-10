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

// we include the library for images loading
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"

// Loader for OpenGL extensions
#include <glad/glad.h>

// class to manage OpenGL failures
#include "gl_check.h"

// GLFW library to create window and to manage I/O
#include <glfw/glfw3.h>

// Utils import
#include <utils/shader.h>   // class to manage shaders 
#include <utils/model.h>    // class to include models
#include <utils/camera.h>   // class to include camera

// Loader for GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/type_ptr.hpp>

// class to manage render-to-texture
#include "framebuffer.h"

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
GLfloat farPillarMaterial[] = {1.0f,1.0f,1.0f};
GLfloat nearPillarMaterial[] = {1.0f,0.5f,0.0f};
GLfloat sphereMaterial[] = {1.0f,1.0f,0.0f};

// variables used to store uniform location inside shaders
GLint pointLightLocation, matDiffuseLocation, matAmbientLocation, matSpecularLocation;  
GLint kdLocation, kaLocation, ksLocation, shineLocation;

// load image from disk and create an OpenGL texture
GLuint LoadTexture(const char* path);

// load the 6 images from disk and create an OpenGL cubemap
GLuint LoadTextureCube(std::string path);

void DrawEnvironment(Shader& shader, Shader& skyboxShader, Model& cubeModel, Model& sphereModel, GLuint skyboxTexture, GLint skyboxLocation, const glm::mat4& view, const glm::mat4& projection);

// variable to manage transition from stage view to virtual view environment & viceversa
bool showEnvironment = false;

// we create a camera. We pass the initial position as a parameter to the constructor. The last boolean tells that we want a camera "anchored" to the ground
Camera camera(glm::vec3(0.0f, 1.7f, 6.0f), GL_TRUE);

// we initialize an array of booleans for each keyboard key
bool keys[1024];

// we need to store the previous mouse position to calculate the offset with the current frame
GLfloat lastX, lastY;

// when rendering the first frame, we do not have a "previous state" for the mouse, so we need to manage this situation
bool firstMouse = true;

// parameters for time calculation
GLfloat deltaTime = 0.0f;
GLfloat lastFrame = 0.0f;
GLfloat currentFrame = 0.0f;

// callback function for keyboard events
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
void fb_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void apply_camera_movements();

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
    
    glfwSetCursorPosCallback(window, mouse_callback);

    // we disable the mouse cursor
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

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
    Shader shader, wallShader, skyboxShader;
    SetupShader(shader, "shaders/scene.vert", "shaders/scene.frag");
    SetupShader(wallShader, "shaders/wall.vert", "shaders/wall.frag");
    SetupShader(skyboxShader, "shaders/skybox.vert", "shaders/skybox.frag");

    // ------- Models -------
    Model planeModel("assets/models/plane.obj");
    Model yodaModel("assets/models/babyyoda.obj");
    Model cubeModel("assets/models/cube.obj");
    Model sphereModel("assets/models/sphere.obj");

    // we set projection and view matrices
    // Projection matrix: FOV angle, aspect ratio, near and far planes
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)screenWidth/(float)screenHeight, 0.1f, 100.0f);
    // View matrix: the camera moves, so we just set to identity now
    glm::mat4 view = glm::mat4(1.0f);

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
    
    // we load the images and store them in a vector
    GLuint wallTexture = LoadTexture("assets/textures/UV_Grid_Sm.png");
    GLint textureLocation = glCheckUniformLocation(wallShader.Program, "wallTexture");

    // off-screen target for the virtual environment 2:1 aspect like the 6x3m wall
    RenderTarget wallTarget(2048, 1024);

    glm::mat4 wallModelMatrix = glm::mat4(1.0f);

    // naive virtual camera: fixed at the wall centre, standard perspective
    glm::mat4 wallViewMatrix = glm::lookAt(glm::vec3(0.0f, 1.5f, -3.0f), glm::vec3(0.0f, 1.5f, -4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 wallProjection = glm::perspective(glm::radians(45.0f), (float)wallTarget.GetWidth() / wallTarget.GetHeight(), 0.1f, 100.0f);

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

    // -------- Skybox -------
    GLuint skyboxTexture = LoadTextureCube("assets/textures/skybox/");
    GLint skyboxLocation = glCheckUniformLocation(skyboxShader.Program, "tCube");

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
        // we determine the time passed from the beginning
        // and we calculate time difference between current frame rendering and the previous one
        currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // Check if an I/O event is happening
        glfwPollEvents();
        // we apply FPS camera movements
        apply_camera_movements();
        // View matrix (=camera): position, view direction, camera "up" vector
        view = camera.GetViewMatrix();

        wallTarget.Bind();
        
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        
        DrawEnvironment(shader, skyboxShader, cubeModel, sphereModel, skyboxTexture, skyboxLocation, wallViewMatrix, wallProjection);
        GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        glfwGetFramebufferSize(window, &width, &height);

        GL_CHECK(glViewport(0, 0, width, height));

        // "clear" the frame and z-buffer
        GL_CHECK(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
        GL_CHECK(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
        

        if(showEnvironment)
            DrawEnvironment(shader, skyboxShader, cubeModel, sphereModel, skyboxTexture, skyboxLocation, view, projection);
        else
        {
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
            yodaModelMatrix = glm::scale(yodaModelMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
            // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
            yodaNormalMatrix = glm::inverseTranspose(glm::mat3(view * yodaModelMatrix));
            GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(yodaModelMatrix)));
            GL_CHECK(glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normals"), 1, GL_FALSE, glm::value_ptr(yodaNormalMatrix)));

            GL_CHECK(glUniform3fv(matDiffuseLocation, 1, yodaMaterial));

            // we render yoda
            yodaModel.Draw();

            // -----------LED WALL-----------

            wallShader.Use();

            // we activate the texture with id 0, and we bind the id to our loaded texture data
            GL_CHECK(glActiveTexture(GL_TEXTURE0));
            GL_CHECK(glBindTexture(GL_TEXTURE_2D, wallTarget.GetColorTexture()));

            GL_CHECK(glUniform1i(textureLocation, 0));

            // we pass projection and view matrices to the Wall Shader
            GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(wallShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(wallModelMatrix)));
            GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(wallShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(view)));
            GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(wallShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection)));

            GL_CHECK(glBindVertexArray(wallVAO));
            GL_CHECK(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0));
            GL_CHECK(glBindVertexArray(0));
        }

        // double buffering: swapping back and front buffers
        glfwSwapBuffers(window);
    }

    // when I exit from the graphics loop, it is because the application is closing
    // we delete the Shader Programs
    DeleteShader(shader);
    DeleteShader(wallShader);
    DeleteShader(skyboxShader);

    GL_CHECK(glDeleteVertexArrays(1, &wallVAO));
    GL_CHECK(glDeleteBuffers(1, &wallVBO));
    GL_CHECK(glDeleteBuffers(1, &wallEBO));

    GL_CHECK(glDeleteTextures(1, &wallTexture));
    GL_CHECK(glDeleteTextures(1, &skyboxTexture));

    // close and delete context
    glfwTerminate();
    return 0;
}

//////////////////////////////////////////
// we load the image from disk and we create an OpenGL texture
GLuint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    stbi_set_flip_vertically_on_load(true);
    image = stbi_load(path, &w, &h, &channels, STBI_rgb);

    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;

    GL_CHECK(glGenTextures(1, &textureImage));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, textureImage));
    
    // 3 channels = RGB ; 4 channel = RGBA
    if (channels==3)
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image));
    else if (channels==4)
        GL_CHECK(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image));
    
    GL_CHECK(glGenerateMipmap(GL_TEXTURE_2D));
    // we set how to consider UVs outside [0,1] range
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    // we set the filtering for minification and magnification
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);

    // we set the binding to 0 once we have finished
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0));

    return textureImage;

}

///////////////////////////////////////////
// load one side of the cubemap, passing the name of the file and the side of the corresponding OpenGL cubemap
void LoadTextureCubeSide(std::string path, std::string side_image, GLenum side_name)
{
    int w, h;
    unsigned char* image;
    std::string fullname;

    // full name and path of the side of the cubemap
    fullname = path + side_image;
    // we load the image file
    stbi_set_flip_vertically_on_load(false);
    image = stbi_load(fullname.c_str(), &w, &h, 0, STBI_rgb);
    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;
    // we set the image file as one of the side of the cubemap (passed as a parameter)
    GL_CHECK(glTexImage2D(side_name, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image));
    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);
}


//////////////////////////////////////////
// we load the 6 images from disk and we create an OpenGL cube map
GLuint LoadTextureCube(std::string path)
{
    GLuint textureImage;

    // we create and activate the OpenGL cubemap texture
    GL_CHECK(glGenTextures(1, &textureImage));
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, textureImage));

    // we load and set the 6 images corresponding to the 6 views of the cubemap
    // we use as convention that the names of the 6 images, placed at the path passed as parameter
    // we load the images individually and we assign them to the correct sides of the cube map
    LoadTextureCubeSide(path, std::string("right.jpg"), GL_TEXTURE_CUBE_MAP_POSITIVE_X);
    LoadTextureCubeSide(path, std::string("left.jpg"), GL_TEXTURE_CUBE_MAP_NEGATIVE_X);
    LoadTextureCubeSide(path, std::string("top.jpg"), GL_TEXTURE_CUBE_MAP_POSITIVE_Y);
    LoadTextureCubeSide(path, std::string("bottom.jpg"), GL_TEXTURE_CUBE_MAP_NEGATIVE_Y);
    LoadTextureCubeSide(path, std::string("front.jpg"), GL_TEXTURE_CUBE_MAP_POSITIVE_Z);
    LoadTextureCubeSide(path, std::string("back.jpg"), GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);

    // we set the filtering for minification and magnification
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    // we set how to consider the texture coordinates outside [0,1] range
    // in this case we have a cube map, so
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    // we set the binding to 0 once we have finished
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

    return textureImage;

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

void DrawEnvironment(Shader& shader, Shader& skyboxShader, Model& cubeModel, Model& sphereModel, GLuint skyboxTexture, GLint skyboxLocation, const glm::mat4& view, const glm::mat4& projection)
{
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

    // ------- VIRTUAL ENVIRONMENT -------

    glm::mat4 farPillarModelMatrix = glm::mat4(1.0f);
    glm::mat3 farPillarNormalMatrix = glm::mat3(1.0f);
    glm::mat4 nearPillarModelMatrix = glm::mat4(1.0f);
    glm::mat3 nearPillarNormalMatrix = glm::mat3(1.0f);
    glm::mat4 sphereModelMatrix = glm::mat4(1.0f);
    glm::mat3 sphereNormalMatrix = glm::mat3(1.0f);

    // -----------FAR PILLAR-----------

    farPillarModelMatrix = glm::translate(farPillarModelMatrix, glm::vec3(0.0f, 3.0f, -15.0f));
    farPillarModelMatrix = glm::scale(farPillarModelMatrix, glm::vec3(1.0f, 3.0f, 1.0f));
    // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
    farPillarNormalMatrix = glm::inverseTranspose(glm::mat3(view * farPillarModelMatrix));
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(farPillarModelMatrix)));
    GL_CHECK(glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normals"), 1, GL_FALSE, glm::value_ptr(farPillarNormalMatrix)));

    GL_CHECK(glUniform3fv(matDiffuseLocation, 1, farPillarMaterial));

    // we render the far pillar
    cubeModel.Draw();

    // -----------NEAR PILLAR-----------
    nearPillarModelMatrix = glm::translate(nearPillarModelMatrix, glm::vec3(-2.0f, 1.5f, -5.0f));
    nearPillarModelMatrix = glm::scale(nearPillarModelMatrix, glm::vec3(0.4f, 1.5f, 0.4f));
    // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
    nearPillarNormalMatrix = glm::inverseTranspose(glm::mat3(view * nearPillarModelMatrix));
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(nearPillarModelMatrix)));
    GL_CHECK(glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normals"), 1, GL_FALSE, glm::value_ptr(nearPillarNormalMatrix)));

    GL_CHECK(glUniform3fv(matDiffuseLocation, 1, nearPillarMaterial));

    // we render the near pillar
    cubeModel.Draw();

    // -----------SPHERE-----------

    sphereModelMatrix = glm::translate(sphereModelMatrix, glm::vec3(1.5f, 1.0f, -8.0f));
    sphereModelMatrix = glm::scale(sphereModelMatrix, glm::vec3(1.0f, 1.0f, 1.0f));
    // if we cast a mat4 to a mat3, we are automatically considering the upper left 3x3 submatrix
    sphereNormalMatrix = glm::inverseTranspose(glm::mat3(view * sphereModelMatrix));
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(shader.Program, "model"), 1, GL_FALSE, glm::value_ptr(sphereModelMatrix)));
    GL_CHECK(glUniformMatrix3fv(glGetUniformLocation(shader.Program, "normals"), 1, GL_FALSE, glm::value_ptr(sphereNormalMatrix)));

    GL_CHECK(glUniform3fv(matDiffuseLocation, 1, sphereMaterial));

    // we render the sphere
    sphereModel.Draw();

    // -----------SKYBOX-----------
    // drawn last: its depth is forced to 1.0 in the vertex shader (xyww),
    // so GL_LEQUAL is needed to let it pass the depth test where nothing else was drawn
    GL_CHECK(glDepthFunc(GL_LEQUAL));

    skyboxShader.Use();

    GL_CHECK(glActiveTexture(GL_TEXTURE0));
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxTexture));
    GL_CHECK(glUniform1i(skyboxLocation, 0));
    // rotation-only view matrix: removing the translation keeps the sky infinitely far away
    glm::mat4 skyView = glm::mat4(glm::mat3(view));

    // we pass projection and view matrices to the Shader Program
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection)));
    GL_CHECK(glUniformMatrix4fv(glGetUniformLocation(skyboxShader.Program, "view"), 1, GL_FALSE, glm::value_ptr(skyView)));

    // we render the skybox
    cubeModel.Draw();

    // restore the default depth test for the next draws
    GL_CHECK(glDepthFunc(GL_LESS));

}

/////////////////// CALLBACK FOR KEYBOARD EVENTS ///////////////////////
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
    // if ESC is pressed, close the application
    if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);

    // if E is pressed, we alternate stage view and virtual view environment
    if(key == GLFW_KEY_E && action == GLFW_PRESS)
        showEnvironment = !showEnvironment;

    // we keep trace of the pressed keys
    // with this method, we can manage 2 keys pressed at the same time:
    // many I/O managers often consider only 1 key pressed at the time (the first pressed, until it is released)
    // using a boolean array, we can then check and manage all the keys pressed at the same time
    if(key < 0 || key >= 1024) 
        return;

    if(action == GLFW_PRESS)
        keys[key] = true;
    else if(action == GLFW_RELEASE)
        keys[key] = false;
}

void fb_size_callback(GLFWwindow* window, int width, int height)
{
    GL_CHECK(glViewport(0, 0, width, height));
}

//////////////////////////////////////////
// If one of the WASD keys is pressed, the camera is moved accordingly (the code is in utils/camera.h)
void apply_camera_movements()
{
    // if a single WASD key is pressed, then we will apply the full value of velocity v in the corresponding direction.
    // However, if two keys are pressed together in order to move diagonally (W+D, W+A, S+D, S+A), 
    // then the camera will apply a compensation factor to the velocities applied in the single directions, 
    // in order to have the full v applied in the diagonal direction  
    // the XOR on A and D is to avoid the application of a wrong attenuation in the case W+A+D or S+A+D are pressed together.  
    GLboolean diagonal_movement = (keys[GLFW_KEY_W] ^ keys[GLFW_KEY_S]) && (keys[GLFW_KEY_A] ^ keys[GLFW_KEY_D]); 
    camera.SetMovementCompensation(diagonal_movement);
    
    if(keys[GLFW_KEY_W])
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if(keys[GLFW_KEY_S])
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if(keys[GLFW_KEY_A])
        camera.ProcessKeyboard(LEFT, deltaTime);
    if(keys[GLFW_KEY_D])
        camera.ProcessKeyboard(RIGHT, deltaTime);
}

//////////////////////////////////////////
// callback for mouse events
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
      // we move the camera view following the mouse cursor
      // we calculate the offset of the mouse cursor from the position in the last frame
      // when rendering the first frame, we do not have a "previous state" for the mouse, so we set the previous state equal to the initial values (thus, the offset will be = 0)
      if(firstMouse)
      {
          lastX = xpos;
          lastY = ypos;
          firstMouse = false;
      }

      // offset of mouse cursor position
      GLfloat xoffset = xpos - lastX;
      GLfloat yoffset = lastY - ypos;

      // the new position will be the previous one for the next frame
      lastX = xpos;
      lastY = ypos;

      // we pass the offset to the Camera class instance in order to update the rendering
      camera.ProcessMouseMovement(xoffset, yoffset);

}
