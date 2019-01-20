#include <Application/Application.h>

int main(int argc, char const* argv[]) {
    try {
        Application app;
        return app.Run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }
}