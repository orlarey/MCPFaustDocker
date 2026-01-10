#include "FaustHelpTool.hh"
#include "utils.hh"
#include <cstdlib>

// Constructor
FaustHelpTool::FaustHelpTool() {
  // Initialize tool-specific data here
}

// Returns the tool name for MCP registration
std::string FaustHelpTool::name() const { return "FaustHelpTool"; }

// Returns the tool description and schema for MCP
std::string FaustHelpTool::describe() const {
  // Build tool description using JSON object
  json description = {
      {"name", name()},
      {"description", "Executes 'faust -h' to retrieve all Faust compiler options and usage information"},
      {"inputSchema",
       {{"type", "object"},
        {"properties", json::object()},
        {"required", json::array()}}}};

  return description.dump();
}

// Executes faust -h and returns help information
json FaustHelpTool::call(const std::string &args) {
  try {
    // Ensure work directory exists
    if (!ensureWorkDir()) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not create work directory"}}});
    }

    // Call Faust via Docker to get help
    auto result = runFaustDocker("-h");

    // Note: faust -h returns non-zero exit code even on success
    // So we don't check the result value here

    // Create path in work directory to store help info
    std::string helpPath = getWorkPath("help.txt");

    // Write help output to file
    std::ofstream helpFile(helpPath);
    helpFile << result.output;
    if (!result.errorOutput.empty()) {
      helpFile << result.errorOutput;
    }
    helpFile.close();

    // Read the contents of help.txt using encodeFile
    auto fileData = encodeFile(helpPath);

    if (!fileData) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not read help file"}}});
    }

    // Return as MCP content array with resource
    json resource = *fileData;

    return json::array({{{"type", "resource"}, {"resource", resource}}});

  } catch (const json::parse_error &e) {
    return json::array(
        {{{"type", "text"}, {"text", "Error: Invalid arguments"}}});
  }
}