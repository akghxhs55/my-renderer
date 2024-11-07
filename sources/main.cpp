#include "my_renderer.h"

#include <iostream>


int main()
{
    try
    {
        MyRenderer app;
        app.run();
    }
    catch (const std::exception& exception)
    {
        std::cerr << exception.what() << std::endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}
