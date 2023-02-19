#include "window.hpp"
#include "GLFW/glfw3.h"
#include "profiler/profiler.hpp"
#include <string>

namespace rot {

RotWindow::RotWindow(int w, int h, std::string name) : width(w), height(h), windowName(name) { initWindow(); }
RotWindow::~RotWindow() {
    PROF_SCOPED(PROF_LVL_ALL, "Window destructor");
    glfwDestroyWindow(window);
    glfwTerminate();
}
void RotWindow::initWindow() {
    PROF_SCOPED(PROF_LVL_ALL, __func__);
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    window = glfwCreateWindow(width, height, windowName.c_str(), nullptr, nullptr);
}

bool RotWindow::shouldClose() { return glfwWindowShouldClose(window); }
} // namespace rot
