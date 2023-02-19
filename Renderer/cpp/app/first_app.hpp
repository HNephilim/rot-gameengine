#pragma once
#include "window/window.hpp"

namespace rot {
class FirstApp {
  public:
    static constexpr int WIDHT = 800;
    static constexpr int HEIGHT = 600;

    void run();

  private:
    RotWindow rotWindow{WIDHT, HEIGHT, "ROT ENGINE =)"};
};
} // namespace rot
