
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

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MediaData, audio, video, images);

void background_worker(std::stop_token token,
  std::atomic<std::shared_ptr<MediaData>> &data,
  const std::unordered_set<std::string_view> &video_ext,
  const std::unordered_set<std::string_view> &audio_ext,
  const std::unordered_set<std::string_view> &image_ext,
  std::int64_t sleep_interval) {
    std::string_view homeDirectory = get_home_directory();
    auto path_to_save = std::filesystem::path(homeDirectory).append(".media_files");
    std::mutex dummy_mtx;
    std::condition_variable_any cv;
    {
        std::unique_lock lock(dummy_mtx);
        while (!token.stop_requested()) {
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
            cv.wait_for(lock, token, std::chrono::seconds(sleep_interval), [] () { return false; });
        }
    }
}

int main(int argc, const char** argv) {
    int64_t timeout = 20;
    if (argc == 2){
        auto [ptr, ec] = std::from_chars(argv[1], argv[1] + std::strlen(argv[1]), timeout);
        if (ec != std::errc{} || *ptr || timeout <= 0) {
            std::cerr << "Invalid timeout format! Must be positive integer!" << std::endl;
            return 1;
        }
    } else if (argc != 1) {
        std::cerr << "Usage: " << argv[0] << " <timeout in secs>" << std::endl;
        return 1;
    }
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
    std::stop_source stop_src;
    std::stop_token token = stop_src.get_token();
    std::jthread t(background_worker, token, std::ref(data), std::ref(video_ext), std::ref(audio_ext), std::ref(image_ext), timeout);
    auto& cors = app.get_middleware<crow::CORSHandler>();
    cors.global().origin("*");
    app.bindaddr("0.0.0.0").port(1234).multithreaded().run();
    stop_src.request_stop();
    t.join();
    return 0;
}
