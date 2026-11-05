#include <catch2/catch.hpp>

#include "rqps/inventory_json.h"

#include <boost/graph/graph_traits.hpp>

TEST_CASE("graph_from_inventory_json parses packages array", "[unit][inventory_json]") {
    const std::string input = R"({
      "packages": [
        { "system": "npm", "name": "left-pad", "version": "1.0.0" }
      ]
    })";
    std::string error;
    const Graph graph = graph_from_inventory_json(input, error);
    CHECK(error.empty());
    CHECK(boost::num_vertices(graph) == 1);
    auto [vertex, end] = boost::vertices(graph);
    CHECK(vertex != end);
    CHECK(graph[*vertex].system == "npm");
    CHECK(graph[*vertex].name == "left-pad");
    CHECK(graph[*vertex].version == "1.0.0");
}

TEST_CASE("graph_from_inventory_json rejects missing packages", "[unit][inventory_json]") {
    std::string error;
    const Graph graph = graph_from_inventory_json("{}", error);
    CHECK(boost::num_vertices(graph) == 0);
    CHECK_FALSE(error.empty());
}

TEST_CASE("inventory_json round trip escapes special characters", "[unit][inventory_json]") {
    Graph graph;
    Package package;
    package.system = "npm";
    package.name = "pkg\"quote";
    package.version = "1.0\n";
    package.action = ActionType::INSTALL;
    boost::add_vertex(package, graph);

    const std::string json = inventory_json_from_graph(graph);
    std::string error;
    const Graph parsed = graph_from_inventory_json(json, error);
    CHECK(error.empty());
    CHECK(boost::num_vertices(parsed) == 1);
    auto [vertex, end] = boost::vertices(parsed);
    CHECK(parsed[*vertex].name == "pkg\"quote");
    CHECK(parsed[*vertex].version == "1.0\n");
}

TEST_CASE("extract_inventory_json_from_discover_response", "[unit][inventory_json]") {
    const std::string discover = R"({
      "path": "/repo",
      "inventory": { "packages": [ { "system": "npm", "name": "a", "version": "1" } ] }
    })";
    const std::string inventory = extract_inventory_json_from_discover_response(discover);
    std::string error;
    const Graph graph = graph_from_inventory_json(inventory, error);
    CHECK(error.empty());
    CHECK(boost::num_vertices(graph) == 1);
}
