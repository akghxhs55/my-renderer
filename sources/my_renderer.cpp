#include "my_renderer.h"

#include <iostream>


MyRenderer::MyRenderer() :
    window(WindowTitle, WindowWidth, WindowHeight),
    environment(ApplicationName, ApplicationVersion, window)
{
}

MyRenderer::~MyRenderer() = default;

void MyRenderer::run()
{
    while (!window.shouldClose())
    {
        glfwPollEvents();
        drawFrame();
    }
}

void MyRenderer::drawFrame()
{
}
