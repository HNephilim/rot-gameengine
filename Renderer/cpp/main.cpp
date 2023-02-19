#include <app/first_app.hpp>
#include <cstdlib>
#include <exception>
#include <iostream>

int main(int argc, char *argv[]) {
    rot::FirstApp app{};

    try {
        app.run();
    } catch (const std::exception &e) {
        std::cerr << e.what() << '\n';
        return 1;
    }
    return 0;
}
