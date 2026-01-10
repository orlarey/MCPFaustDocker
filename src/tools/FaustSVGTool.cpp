#include "FaustSVGTool.hh"
#include "utils.hh"

// Constructor
FaustSVGTool::FaustSVGTool() {
  // Initialize tool-specific data here
}

// Returns the tool name for MCP registration
std::string FaustSVGTool::name() const { return "FaustSVGTool"; }

// Returns the tool description and schema for MCP
std::string FaustSVGTool::describe() const {
  // Build tool description using JSON object
  json description = {
      {"name", name()},
      {"description", "Compiles Faust DSP code and generates SVG block diagram "
                      "visualization using 'faust -svg'"},
      {"inputSchema",
       {{"type", "object"},
        {"properties",
         {{"value",
           {{"type", "string"},
            {"description", "Faust DSP source code to compile and visualize as "
                            "SVG diagram"}}}}},
        {"required", json::array({"value"})}}}};

  return description.dump();
}

// Generates SVG diagram from Faust DSP code
json FaustSVGTool::call(const std::string &args) {
  try {
    // Ensure work directory exists
    if (!ensureWorkDir()) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not create work directory"}}});
    }

    // Parse the JSON arguments
    json arguments = json::parse(args);

    // Extract the 'value' field
    std::string srcCode = arguments.value("value", "process = _;");

    // Create paths in work directory
    std::string dspPath = getWorkPath("source.dsp");
    std::string errPath = getWorkPath("source.txt");
    std::string svgPath = getWorkPath("source-svg/process.svg");

    // Store the faust code into file
    std::ofstream outFile(dspPath);
    outFile << srcCode;
    outFile.close();

    // Call the Faust compiler via Docker to generate SVG
    // SVG files are created in a subdirectory named source-svg/
    auto result = runFaustDocker("-o /dev/null -svg /tmp/source.dsp");

    if (result.exitCode != 0) {
      // Write stderr to error file
      std::ofstream errFile(errPath);
      errFile << result.errorOutput;
      errFile.close();

      // Read error file if compilation failed
      auto errData = encodeFile(errPath);
      if (errData) {
        json resource = *errData;
        return json::array({{{"type", "resource"}, {"resource", resource}}});
      }
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Failed to execute faust command"}}});
    }
    // Read the generated SVG file
    auto fileData = encodeFile(svgPath);

    if (!fileData) {
      return json::array(
          {{{"type", "text"}, {"text", "Error: Could not read SVG file"}}});
    }

    // Return as MCP content array with resource
    json resource = *fileData;

    return json::array({{{"type", "resource"}, {"resource", resource}}});

  } catch (const json::parse_error &e) {
    // Handle parse error
    return json::array(
        {{{"type", "text"}, {"text", "Error: Invalid arguments"}}});
  }
}