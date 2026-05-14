
#include <atomic>
#include <filesystem>
#include <fstream>
#include <ios>
#include <memory>
#include <sstream>
#include <string>

#include <crow.h>
#include <crow/common.h>
#include <crow/middlewares/cors.h>
#include "file_walker.h"
#include "json.hpp"

#include "media_data.h"

std::atomic<bool> running{true};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MediaData, audio, video, images);

void background_worker(std::atomic<std::shared_ptr<MediaData>> &data,
  std::unordered_set<std::string_view> video_ext,
  std::unordered_set<std::string_view> audio_ext,
  std::unordered_set<std::string_view> image_ext,
  std::int64_t sleep_interval) {
    std::string_view homeDirectory = get_home_directory();
    auto path_to_save = std::filesystem::path(homeDirectory).append(".media_files");
    while (running.load(std::memory_order_acquire)) {
        std::this_thread::sleep_for(std::chrono::seconds(sleep_interval));
        auto d = file_walk(data, video_ext, audio_ext, image_ext, homeDirectory);
        std::ofstream file(path_to_save.c_str());
        if (!file) {
            std::cerr << "Error while opening file!" << std::endl;
            continue;
        }
        file << static_cast<nlohmann::json>(*d);
        if (!file) {
            std::cerr << "Error while writing to file!" << std::endl;
        }
    }
}

int main(int /*argc*/, const char** /*argv*/) {
    crow::App<crow::CORSHandler> app;

    std::atomic<std::shared_ptr<MediaData>> data = std::make_shared<MediaData>();
    std::unordered_set<std::string_view> video_ext { ".mp4", ".avi", ".mov", ".mkv", ".webm", ".flv", ".wmv", ".m4v", ".mpg", ".mpeg", ".3gp", ".ogv", ".ts", ".m2ts", ".vob", ".divx", ".xvid" };
    std::unordered_set<std::string_view> audio_ext { ".mp3", ".wav", ".ogg", ".flac", ".aac", ".wma", ".m4a", ".aiff", ".aif", ".opus", ".mid", ".midi" };
    std::unordered_set<std::string_view> image_ext { ".jpg", ".jpeg", ".png", ".gif", ".bmp", ".webp", ".svg", ".ico", ".tiff", ".tif" };

    CROW_ROUTE(app, "/media_files")
        .methods(crow::HTTPMethod::Get)([&data]() {
            std::stringstream s;
            s << static_cast<nlohmann::json>(*data.load(std::memory_order_acquire));
            return crow::response(s.str());
        });
    std::thread t(background_worker, data, video_ext, audio_ext, image_ext, 1);
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global().origin("*");
    app.bindaddr("0.0.0.0").port(1234).multithreaded().run();
    running.store(false, std::memory_order_release);
    t.join();
    return 0;
}
