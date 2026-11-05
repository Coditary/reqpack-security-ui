#include "rqps/remote_config.h"

#include "rqps/rqps_config.h"

#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <httplib.h>

#include <fstream>

std::filesystem::path remote_store_path() {
    return rqps_default_config_dir() / "remotes.json";
}

RemoteStore load_remote_store() {
    RemoteStore store;
    const std::filesystem::path path = remote_store_path();
    if (!std::filesystem::exists(path)) {
        return store;
    }
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(path.string(), tree);
    store.defaultRemote = tree.get("defaultRemote", "");
    if (const auto remotes = tree.get_child_optional("remotes")) {
        for (const auto& [name, urlNode] : remotes.value()) {
            store.remotes[name] = urlNode.get_value<std::string>();
        }
    }
    return store;
}

void save_remote_store(const RemoteStore& store) {
    const std::filesystem::path path = remote_store_path();
    std::filesystem::create_directories(path.parent_path());
    boost::property_tree::ptree tree;
    tree.put("defaultRemote", store.defaultRemote);
    boost::property_tree::ptree remotes;
    for (const auto& [name, url] : store.remotes) {
        remotes.put(name, url);
    }
    tree.add_child("remotes", remotes);
    boost::property_tree::write_json(path.string(), tree);
}

std::optional<std::string> remote_ping(const std::string& baseUrl) {
    httplib::Client client(baseUrl);
    client.set_connection_timeout(5, 0);
    client.set_read_timeout(10, 0);
    const auto response = client.Get("/api/v1/health");
    if (!response) {
        return std::string("connection failed");
    }
    if (response->status != 200) {
        return std::string("HTTP ") + std::to_string(response->status);
    }
    return std::nullopt;
}
