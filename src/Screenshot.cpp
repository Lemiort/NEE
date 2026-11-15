#include "Screenshot.hpp"

#include <glad/gl.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <ctime>
#include <expected>
#include <filesystem>
#include <format>
#include <fstream>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

namespace {
constexpr int kChannels = 3;
constexpr int kJpegQuality = 95;

// stb writes through this callback so paths with any characters work on
// every platform, which stbi_write_*(const char*) does not guarantee.
void WriteToStream(void* context, void* data, int size) {
    static_cast<std::ofstream*>(context)->write(static_cast<const char*>(data),
                                                size);
}

std::string LowercaseExtension(const std::filesystem::path& path) {
    std::string extension = path.extension().string();
    std::ranges::transform(extension, extension.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return extension;
}
}  // namespace

std::expected<void, std::string> SaveScreenshot(
    const std::filesystem::path& path, int width, int height) {
    if (width <= 0 || height <= 0) {
        return std::unexpected(
            std::format("Cannot capture an empty {}x{} image", width, height));
    }
    const std::string extension = LowercaseExtension(path);
    const bool png = extension == ".png";
    if (!png && extension != ".jpg" && extension != ".jpeg") {
        return std::unexpected(std::format(
            "Unsupported screenshot format '{}'; use .png or .jpg", extension));
    }

    std::vector<std::uint8_t> pixels(static_cast<std::size_t>(width) *
                                     static_cast<std::size_t>(height) *
                                     kChannels);
    GLint previousAlignment = 4;
    glGetIntegerv(GL_PACK_ALIGNMENT, &previousAlignment);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, pixels.data());
    glPixelStorei(GL_PACK_ALIGNMENT, previousAlignment);

    // OpenGL rows start at the bottom; image files start at the top.
    const std::size_t rowBytes = static_cast<std::size_t>(width) * kChannels;
    for (std::size_t top = 0, bottom = static_cast<std::size_t>(height) - 1;
         top < bottom; ++top, --bottom) {
        std::swap_ranges(pixels.begin() + static_cast<std::ptrdiff_t>(top * rowBytes),
                         pixels.begin() + static_cast<std::ptrdiff_t>((top + 1) * rowBytes),
                         pixels.begin() + static_cast<std::ptrdiff_t>(bottom * rowBytes));
    }

    std::error_code directoryError;
    if (path.has_parent_path()) {
        std::filesystem::create_directories(path.parent_path(), directoryError);
    }
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return std::unexpected(
            std::format("Cannot create '{}'", path.string()));
    }
    const int written =
        png ? stbi_write_png_to_func(&WriteToStream, &file, width, height,
                                     kChannels, pixels.data(),
                                     static_cast<int>(rowBytes))
            : stbi_write_jpg_to_func(&WriteToStream, &file, width, height,
                                     kChannels, pixels.data(), kJpegQuality);
    file.close();
    if (written == 0 || !file) {
        return std::unexpected(
            std::format("Cannot write '{}'", path.string()));
    }
    return {};
}

std::expected<std::filesystem::path, std::string> TimestampedScreenshotPath(
    const std::filesystem::path& directory, std::string_view extension) {
    const std::time_t now = std::time(nullptr);
    if (now == static_cast<std::time_t>(-1)) {
        return std::unexpected("Unable to read the current time");
    }
    std::tm local{};
#ifdef _WIN32
    if (localtime_s(&local, &now) != 0) {
#else
    if (localtime_r(&now, &local) == nullptr) {
#endif
        return std::unexpected("Unable to convert the current time");
    }
    std::array<char, 32> stamp{};
    if (std::strftime(stamp.data(), stamp.size(), "%Y-%m-%d %H.%M.%S",
                      &local) == 0) {
        return std::unexpected("Unable to format the current time");
    }
    return directory /
           std::format("Screenshot {}{}", stamp.data(), extension);
}
