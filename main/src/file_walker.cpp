#include "file_walker.h"

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

#include <iostream>
#include <stdexcept>
#include <filesystem>
#include <string_view>

std::shared_ptr<MediaData> file_walk(std::atomic<std::shared_ptr<MediaData>> &data,
  const std::unordered_set<std::string_view> &video_ext,
  const std::unordered_set<std::string_view> &audio_ext,
  const std::unordered_set<std::string_view> &image_ext,
  const std::string_view &homeDirectory) {
    namespace fs = std::filesystem;
    fs::recursive_directory_iterator it(homeDirectory,
        fs::directory_options::skip_permission_denied
    );
    std::shared_ptr<MediaData> d = std::make_shared<MediaData>();
    try {
        for (auto& entry : it) {
            if (entry.is_regular_file()) {
                const char *ext = entry.path().extension().c_str();
                if (video_ext.contains(ext)) {
                    d->video.push_back(entry.path().filename());
                } else if (audio_ext.contains(ext)) {
                    d->audio.push_back(entry.path().filename());
                } else if (image_ext.contains(ext)) {
                    d->images.push_back(entry.path().filename());
                }
            }
        }
    } catch (const fs::filesystem_error &e) {
        std::cerr << "Error while directory pass: " << e.what() << std::endl;
    }
    data.store(d, std::memory_order_release);
    return d;
}

std::string_view get_home_directory() {
    uid_t uid = getuid();
    struct passwd* pw = getpwuid(uid);
    if (pw && pw->pw_dir) {
        return std::string_view(pw->pw_dir);
    }
    throw std::runtime_error("Home directory can't be found");
}