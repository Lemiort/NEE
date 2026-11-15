#ifndef SCREENSHOT_HPP_INCLUDED
#define SCREENSHOT_HPP_INCLUDED

#include <expected>
#include <filesystem>
#include <string>
#include <string_view>

// Saves the color buffer of the framebuffer bound for reading
// (GL_READ_FRAMEBUFFER; 0 is the window) as an image file. Reads the
// rectangle from (0, 0) to (width, height), flipped so the image's first row is
// the top of the screen. The format follows the extension: .png or
// .jpg/.jpeg. Missing parent directories are created. Requires a current
// OpenGL context; pixel pack state is restored afterwards.
[[nodiscard]] std::expected<void, std::string> SaveScreenshot(
    const std::filesystem::path& path, int width, int height);

// "<directory>/Screenshot 2026-11-15 21.42.07<extension>" for the current
// local time.
[[nodiscard]] std::expected<std::filesystem::path, std::string>
TimestampedScreenshotPath(const std::filesystem::path& directory,
                          std::string_view extension = ".png");

#endif  // SCREENSHOT_HPP_INCLUDED
