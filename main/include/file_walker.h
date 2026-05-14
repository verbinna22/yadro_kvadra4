#ifndef FILE_WALKER_H
#define FILE_WALKER_H

#include <atomic>
#include <memory>
#include <unordered_set>

#include "media_data.h"

std::shared_ptr<MediaData> file_walk(std::atomic<std::shared_ptr<MediaData>> &data,
  const std::unordered_set<std::string_view> &video_ext,
  const std::unordered_set<std::string_view> &audio_ext,
  const std::unordered_set<std::string_view> &image_ext,
  std::string_view homeDirectory);

std::string_view get_home_directory();

#endif // FILE_WALKER_H
