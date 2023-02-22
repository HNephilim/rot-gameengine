#pragma once

#include <string>
#include <vector>
namespace rot {
class RotPipeline {
  public:
    RotPipeline(const std::string &vertFilepath, const std::string &fragFilePath);

  private:
    static std::vector<char> readFile(const std::string &filepath);

    void createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilePath);
};
} // namespace rot
