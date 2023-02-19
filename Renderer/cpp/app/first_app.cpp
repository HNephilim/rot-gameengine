#include "first_app.hpp"
#include "GLFW/glfw3.h"
#include "window/window.hpp"

namespace rot {
void FirstApp::run() {
    while (!rotWindow.shouldClose()) {
        glfwPollEvents();
    }
}
} // namespace rot
