#pragma once
#include "pipeline/pipeline.hpp"
#include "window/window.hpp"

namespace rot {
class FirstApp {
  public:
    static constexpr int WIDHT = 800;
    static constexpr int HEIGHT = 600;

    void run();

  private:
    RotWindow rotWindow{WIDHT, HEIGHT, "ROT ENGINE =)"};
    RotPipeline rotPipeline{"../shaders/simple_shader.frag.spv", "../shaders/simple_shader.vert.spv"};
};
} // namespace rot
