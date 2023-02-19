#pragma once
#include <string>
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace rot {

class RotWindow {
  public:
    RotWindow(int w, int h, std::string name);
    ~RotWindow();

    // Adhering to RAII, we should delete copy constructors to guard the GLFWwindow pointer;
    RotWindow(const RotWindow &) = delete;
    RotWindow &operator=(const RotWindow &) = delete;

    bool shouldClose();

  private:
    void initWindow();

    const int width;
    const int height;

    std::string windowName;
    GLFWwindow *window;
};

} // namespace rot
