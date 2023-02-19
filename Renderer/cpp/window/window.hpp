#pragma once
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace rot {

class RotWindow {
  private:
    void initWindow();

    GLFWwindow *window;
};

} // namespace rot
