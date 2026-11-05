#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>

struct RemoteProfile {
    std::string name;
    std::string baseUrl;
};

struct RemoteStore {
    std::string defaultRemote;
    std::map<std::string, std::string> remotes;
};

RemoteStore load_remote_store();

void save_remote_store(const RemoteStore& store);

std::filesystem::path remote_store_path();

std::optional<std::string> remote_ping(const std::string& baseUrl);
