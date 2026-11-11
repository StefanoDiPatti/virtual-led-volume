/*
Texture loading functions
- LoadTexture: 2D texture from an image file (flipped vertically, with mipmaps)
- LoadTextureCube: cubemap from the 6 images of a folder (right, left, top, bottom, front, back)

N.B.) the stb_image implementation (STB_IMAGE_IMPLEMENTATION) is compiled only once, in main.cpp
*/

#pragma once

// Std. Includes
#include <string>
#include <iostream>

// Loader for OpenGL extensions
#include <glad/glad.h>

// library for images loading
#include <stb_image/stb_image.h>

// class to manage OpenGL failures
#include <render/gl_check.h>

// we load the image from disk and we create an OpenGL texture
inline GLuint LoadTexture(const char* path)
{
    GLuint textureImage;
    int w, h, channels;
    unsigned char* image;
    // image files store the top row first, while OpenGL places the UV origin at the bottom-left
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

// load one side of the cubemap, passing the name of the file and the side of the corresponding OpenGL cubemap
inline void LoadTextureCubeSide(const std::string& path, const std::string& side_image, GLenum side_name)
{
    int w, h;
    unsigned char* image;
    std::string fullname;

    // full name and path of the side of the cubemap
    fullname = path + side_image;
    // we load the image file (cubemap faces are not flipped)
    stbi_set_flip_vertically_on_load(false);
    image = stbi_load(fullname.c_str(), &w, &h, 0, STBI_rgb);
    if (image == nullptr)
        std::cout << "Failed to load texture!" << std::endl;
    // we set the image file as one of the side of the cubemap (passed as a parameter)
    GL_CHECK(glTexImage2D(side_name, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, image));
    // we free the memory once we have created an OpenGL texture
    stbi_image_free(image);
}

// we load the 6 images from disk and we create an OpenGL cube map
inline GLuint LoadTextureCube(const std::string& path)
{
    GLuint textureImage;

    // we create and activate the OpenGL cubemap texture
    GL_CHECK(glGenTextures(1, &textureImage));
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, textureImage));

    // we load and set the 6 images corresponding to the 6 views of the cubemap
    // the images (right, left, top, bottom, front, back) are placed at the path passed as parameter
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
    // in this case we have a cube map, so we need to set also the third coordinate (R)
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
    GL_CHECK(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE));

    // we set the binding to 0 once we have finished
    GL_CHECK(glBindTexture(GL_TEXTURE_CUBE_MAP, 0));

    return textureImage;
}
