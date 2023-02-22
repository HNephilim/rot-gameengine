#include "pipeline.hpp"

#include "profiler/profiler.hpp"
#include <fstream>
#include <ios>
#include <iostream>
#include <stdexcept>
#include <stdint.h>
#include <vector>

namespace rot {

RotPipeline::RotPipeline(const std::string &vertFilepath, const std::string &fragFilePath) {
    PROF_SCOPED(PROF_LVL_ALL, __func__);
    createGraphicsPipeline(vertFilepath, fragFilePath);
}

std::vector<char> RotPipeline::readFile(const std::string &filePath) {
    PROF_SCOPED(PROF_LVL_ALL, __func__, "path=" + filePath);

    // Open file
    std::ifstream file(filePath, std::ios::ate | std::ios::binary);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file :" + filePath);
    }

    // Ingesting buffer

    // We openned "ate" (At the end). We can get the file size by looking at de cursor position
    // We'll use it to initialize a byte-buffer
    size_t fileSize = static_cast<size_t>(file.tellg());
    std::vector<char> buffer(fileSize);

    // return cursor to beginning so we can read it to de buffer
    file.seekg(0);
    file.read(buffer.data(), fileSize);

    return buffer;
}

void RotPipeline::createGraphicsPipeline(const std::string &vertFilepath, const std::string &fragFilePath) {
    PROF_SCOPED(PROF_LVL_ALL, __func__);

    auto vertCode = readFile(vertFilepath);
    auto fragCode = readFile(fragFilePath);

    std::cout << "Vertex Shader Code Size: " << vertCode.size() << '\n';
    std::cout << "Fragment Shader Code Size: " << fragCode.size() << '\n';
}

} // namespace rot
