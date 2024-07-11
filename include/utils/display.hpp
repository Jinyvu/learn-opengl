#ifndef DISPLAY_ON_SCREEN_H
#define DISPLAY_ON_SCREEN_H

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <utils/callback_manager.hpp>
#include <utils/screen_capture.hpp>
#include <map>
#include <memory>

#include <iostream>
#include <stdexcept>

struct FrameInfoStruct
{
    unsigned int width;
    unsigned int height;
    unsigned int frame;
    float deltaTime;
    float time;
};

class Display
{
public:
    unsigned int width;
    unsigned int height;
    int framebufferWidth;
    int framebufferHeight;

    Display(unsigned int width = 800, unsigned int height = 600);
    ~Display();
    void on(const char *event, const CallbackManager<FrameInfoStruct>::Callback &callback);
    void render();
    void pause();
    void resume();
    void turnOnCapture(EScreenCaptureFormat format, const char *outputPath, unsigned int frameRate = 24);
    void tunrnDownCapture();

private:
    // display
    float deltaTime;
    float lastFrame;
    unsigned int frame;
    bool shouldPause;
    GLFWwindow *window;

    // capture
    bool enableCapture;
    std::unique_ptr<ScreenCapture> screenCapture;

    // callbacks
    std::map<const char *, std::unique_ptr<CallbackManager<FrameInfoStruct>>> callbacksMap;

    void close();
    static void framebuffer_size_callback(GLFWwindow *window, int width, int height);
    void processInput(GLFWwindow *window);
};

Display::Display(unsigned int width, unsigned int height)
    : width(width), height(height), framebufferWidth(0), framebufferHeight(0), deltaTime(0.0), lastFrame(0.0), 
    frame(0), shouldPause(false), window(nullptr), enableCapture(false), screenCapture(nullptr)
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    window = glfwCreateWindow(width, height, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        glfwTerminate();
        throw std::runtime_error("Failed to create GLFW window.");
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, Display::framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD.");
    }

    // on retina device, actual framebuffer size may larger than logical size
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
}

Display::~Display()
{
    close();
}

void Display::on(const char *event, const CallbackManager<FrameInfoStruct>::Callback &callback)
{
    auto it = callbacksMap.find(event);
    if (it == callbacksMap.end())
    {
        callbacksMap[event] = std::make_unique<CallbackManager<FrameInfoStruct>>();
    }

    it = callbacksMap.find(event);
    it->second->registerCallback(callback);
}

void Display::render()
{
    while (!glfwWindowShouldClose(window) && !shouldPause)
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        const FrameInfoStruct frameInfo{
            width,
            height,
            frame,
            deltaTime,
            lastFrame,
        };
        frame++;

        processInput(window);
        // run all the render event
        if (!callbacksMap.empty())
        {
            auto it = callbacksMap.find("render");
            if (it != callbacksMap.end())
            {
                it->second->invoke(frameInfo);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Display::close()
{
    if (!callbacksMap.empty())
    {
        auto it = callbacksMap.find("close");
        if (it != callbacksMap.end())
        {
            const FrameInfoStruct frameInfo{
                width,
                height,
                frame,
                deltaTime,
                lastFrame};
            it->second->invoke(frameInfo);
        }
    }

    glfwTerminate();
}

void Display::pause()
{
    shouldPause = true;
}

void Display::resume()
{
    shouldPause = false;
    render();
}

void Display::turnOnCapture(EScreenCaptureFormat format, const char *outputPath, unsigned int frameRate)
{
    if (screenCapture == nullptr)
    {
        screenCapture = std::make_unique<ScreenCapture>(framebufferWidth, framebufferHeight);
    }
    screenCapture->openOutputContext(outputPath);

    on("render",
       [&](auto _)
       {
           uint8_t *data = new uint8_t[framebufferWidth * framebufferHeight * 3];
           glReadPixels(0, 0, framebufferWidth, framebufferHeight, GL_RGB, GL_UNSIGNED_BYTE, data);
           screenCapture->encodeFrame(data);
           delete[] data;
       });
}

void Display::tunrnDownCapture()
{
}

void Display::framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void Display::processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

#endif