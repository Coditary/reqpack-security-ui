#pragma once

#include "rqp/security/security_types.h"

#include <string>
#include <vector>

Graph graph_from_inventory_json(const std::string& inputJson, std::string& error);

std::string inventory_json_from_graph(const Graph& graph);

std::string extract_inventory_json_from_discover_response(const std::string& discoverJson);
