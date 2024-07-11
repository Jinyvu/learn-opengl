#include <glad/glad.h>
#include <iostream>
#include <loader/shader.h>
#include <loader/texture.h>
#include <filesystem>
#include <utils/display.hpp>

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
const float SCALE = 0.9;

// path
const std::filesystem::path CUR_DIR_PATH = std::filesystem::current_path() / "../src/test/displayTest";
const std::filesystem::path RESOURCES_DIR_PATH = std::filesystem::current_path() / "../resources";
const std::filesystem::path OUTPUT_DIR_PATH = std::filesystem::current_path() / "../output";

int main()
{
    Display display(SCR_WIDTH, SCR_HEIGHT);

    display.turnOnCapture(EScreenCaptureFormat_MP4, (OUTPUT_DIR_PATH / "videos/test.mp4").c_str());

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 将shader编译结果链接到一个shader program，前一个shader的输出会作为下一个shader的输入
    Shader ourShader((CUR_DIR_PATH / "shaders/vertex.glsl").c_str(), (CUR_DIR_PATH / "shaders/fragment.glsl").c_str());

    // 加载并绑定纹理
    TextureInfo textureInfo1 = loadTexture((RESOURCES_DIR_PATH / "productions/cloth1.png").c_str(), GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);
    TextureInfo bgTextureInfo = loadTexture((RESOURCES_DIR_PATH / "textures/wood.png").c_str(), GL_CLAMP_TO_BORDER, GL_CLAMP_TO_BORDER);

    // 顶点数据
    float vertices[] = {
        // positions          // colors           // texture coords
        1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,   // top right
        1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,  // bottom right
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, // bottom left
        -1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f   // top left
    };
    unsigned int indices[] = {
        // note that we start from 0!
        0, 1, 3, // first Triangle
        1, 2, 3  // second Triangle
    };

    // apply scale
    for (unsigned int i = 0; i < 4; i++)
    {
        vertices[i * 8] *= SCALE;
        vertices[i * 8 + 1] *= SCALE;
        vertices[i * 8 + 2] *= SCALE;
        vertices[i * 8 + 3] *= SCALE;
    }

    // 向GPU申请一块buffer
    unsigned int VBO, VAO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // 指定buffer应该以何种形式被消费
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    // 从内存拷贝数据到GPU中的buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    // color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    // texture
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void *)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // // 解除VBO到GL_ARRAY_BUFFER的绑定
    // glBindBuffer(GL_ARRAY_BUFFER, 0);
    // // 解除VAO到vertexArray的绑定
    // glBindVertexArray(0);

    ourShader.use();
    ourShader.setInt("texture1", 0);
    ourShader.setInt("textureBg", 1);
    ourShader.setVec2("resolution", textureInfo1.width, textureInfo1.height);

    display.on("render",
               [&](FrameInfoStruct frameInfo)
               {
                   ourShader.setFloat("time", frameInfo.time);
                   // refresh
                   glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
                   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                   glActiveTexture(GL_TEXTURE0);
                   glBindTexture(GL_TEXTURE_2D, textureInfo1.textureID);
                   glActiveTexture(GL_TEXTURE1);
                   glBindTexture(GL_TEXTURE_2D, bgTextureInfo.textureID);

                   // render
                   ourShader.use();
                   glBindVertexArray(VAO);
                   // glDrawArrays(GL_TRIANGLES, 0, 3);
                   glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
               });

    display.on("close",
               [&](auto _)
               {
                   glDeleteVertexArrays(1, &VAO);
                   glDeleteBuffers(1, &VBO);
                   glDeleteBuffers(1, &EBO);
               });

    display.render();

    return 0;
}