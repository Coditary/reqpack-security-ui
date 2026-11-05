#include "rqps/inventory_json.h"

#include "rqps/json_util.h"

#include <boost/graph/graph_traits.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/property_tree/ptree.hpp>

#include <sstream>

Graph graph_from_inventory_json(const std::string& inputJson, std::string& error) {
    Graph graph;
    try {
        std::istringstream stream(inputJson);
        boost::property_tree::ptree tree;
        boost::property_tree::read_json(stream, tree);

        const auto packages = tree.get_child_optional("packages");
        if (!packages.has_value()) {
            error = "expected top-level \"packages\" array";
            return graph;
        }

        for (const auto& [_, entry] : packages.value()) {
            Package package;
            package.action = ActionType::INSTALL;
            package.system = entry.get("system", "");
            package.name = entry.get("name", "");
            package.version = entry.get("version", "");
            boost::add_vertex(package, graph);
        }
    } catch (const std::exception& exception) {
        error = exception.what();
    }
    return graph;
}

std::string inventory_json_from_graph(const Graph& graph) {
    std::ostringstream stream;
    stream << "{\"packages\":[";
    bool first = true;
    auto [vertex, end] = boost::vertices(graph);
    for (; vertex != end; ++vertex) {
        const Package& package = graph[*vertex];
        if (!first) {
            stream << ',';
        }
        first = false;
        stream << '{'
               << "\"system\":\"" << json_escape(package.system) << "\","
               << "\"name\":\"" << json_escape(package.name) << "\","
               << "\"version\":\"" << json_escape(package.version) << "\""
               << '}';
    }
    stream << "]}";
    return stream.str();
}

std::string extract_inventory_json_from_discover_response(const std::string& discoverJson) {
    std::istringstream stream(discoverJson);
    boost::property_tree::ptree tree;
    boost::property_tree::read_json(stream, tree);
    std::ostringstream inventory;
    boost::property_tree::write_json(inventory, tree.get_child("inventory"));
    return inventory.str();
}
