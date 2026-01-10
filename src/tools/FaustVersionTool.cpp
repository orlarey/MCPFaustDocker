#include "FaustVersionTool.hh"
#include "utils.hh"
#include <cstdlib>

// Constructor
FaustVersionTool::FaustVersionTool() {
  // Initialize tool-specific data here
}

// Returns the tool name for MCP registration
std::string FaustVersionTool::name() const { return "FaustVersionTool"; }

// Returns the tool description and schema for MCP
std::string FaustVersionTool::describe() const {
  // Build tool description using JSON object
  json description = {
      {"name", name()},
      {"description", "Executes 'faust -v' to retrieve Faust compiler version and build information"},
      {"inputSchema",
       {{"type", "object"},
        {"properties", json::object()},
        {"required", json::array()}}}};

  return description.dump();
}

// Executes faust -v and returns version information
json FaustVersionTool::call(const std::string &args) {
  try {
    // Ensure work directory exists
    if (!ensureWorkDir()) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not create work directory"}}});
    }

    // Call Faust via Docker to get version
    auto result = runFaustDocker("-v");

    if (result.exitCode != 0) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Failed to execute faust command"}}});
    }

    // Create path in work directory to store version info
    std::string versionPath = getWorkPath("version.txt");

    // Write version output to file
    std::ofstream versionFile(versionPath);
    versionFile << result.output;
    if (!result.errorOutput.empty()) {
      versionFile << result.errorOutput;
    }
    versionFile.close();

    // Read the contents of version.txt using encodeFile
    auto fileData = encodeFile(versionPath);

    if (!fileData) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not read version.txt file"}}});
    }

    // Return as MCP content array with resource
    // Merge encodeFile result with URI
    json resource = *fileData;
    // resource["uri"] = "file://version.txt";

    return json::array({{{"type", "resource"}, {"resource", resource}}});

  } catch (const json::parse_error &e) {
    return json::array(
        {{{"type", "text"}, {"text", "Error: Invalid arguments"}}});
  }
}