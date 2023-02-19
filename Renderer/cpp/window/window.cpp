#include "window.hpp"
#include "GLFW/glfw3.h"
#include <string>

namespace rot {

RotWindow::RotWindow(int w, int h, std::string name) : width(w), height(h), windowName(name) { initWindow(); }
RotWindow::~RotWindow() {
    glfwDestroyWindow(window);
    glfwTerminate();
}
void RotWindow::initWindow() {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    this->window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

} // namespace rot
