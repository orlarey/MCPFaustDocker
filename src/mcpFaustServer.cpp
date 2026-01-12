#include <iostream>
#include <memory>

#include "FaustCompileTool.hh"
#include "FaustVersionTool.hh"
#include "FaustSVGTool.hh"
#include "FaustHelpTool.hh"
#include "FaustSpectrogramTool.hh"
#include "json.hpp"
#include "mcpServer.hh"

using json = nlohmann::json;

// Main entry point - creates and runs the MCP Faust server
int main() {
  SimpleMCPServer server("mcpFaustServer");
  server.registerTool(std::make_unique<FaustVersionTool>());
  server.registerTool(std::make_unique<FaustCompileTool>());
  server.registerTool(std::make_unique<FaustSVGTool>());
  server.registerTool(std::make_unique<FaustHelpTool>());
  server.registerTool(std::make_unique<FaustSpectrogramTool>());
  server.run();
  return 0;
}