

#include "ToyOpenGL.hpp"
#include <iostream>

int main(int argc, char const *argv[])
{
    ToyOpenGLApp toy = ToyOpenGLApp(
        fs::path{std::string{argv[0]}}, 1280, 720, std::string{argv[1]}, std::string{argv[2]}, "");
    int returnCode = toy.run();
    return returnCode;
}
