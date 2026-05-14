#ifndef MEDIA_DATA_H
#define MEDIA_DATA_H

#include <string>
#include <vector>

struct MediaData {
    std::vector<std::string> audio;
    std::vector<std::string> video;
    std::vector<std::string> images;
};

#endif // MEDIA_DATA_H
