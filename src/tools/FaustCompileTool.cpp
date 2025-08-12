#include "FaustCompileTool.hh"
#include "utils.hh"

// Constructor
FaustCompileTool::FaustCompileTool() {
  // Initialize tool-specific data here
}

// Returns the tool name for MCP registration
std::string FaustCompileTool::name() const { return "FaustCompileTool"; }

// Returns the tool description and schema for MCP
std::string FaustCompileTool::describe() const {
  // Build tool description using JSON object
  json description = {
      {"name", name()},
      {"description", "A tool to compile Faust programs"},
      {"inputSchema",
       {{"type", "object"},
        {"properties",
         {{"value",
           {{"type", "string"},
            {"description", "Faust source code to compile"}}},
          {"filename",
           {{"type", "string"},
            {"description", "The filename of the Faust source code"}}},
          {"options",
           {{"type", "string"},
            {"description", "Optional compilation options"}}}}},
        {"required", json::array({"value"})}}}};

  return description.dump();
}

// Compiles Faust DSP code to C++ and returns the result
json FaustCompileTool::call(const std::string &args) {
  try {
    // Ensure work directory exists
    if (!ensureWorkDir()) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not create work directory"}}});
    }

    // Parse the JSON arguments
    json arguments = json::parse(args);

    // Extract the 'value' field (the code) and the compilation options
    std::string srcCode = arguments.value("value", "process = _;");
    std::string compileOptions = arguments.value("options", "");

    // filenames to use for output files (in work directory)
    std::string dspfilename = arguments.value("filename", "source.dsp");
    std::string baseName = dspfilename.substr(0, dspfilename.find_last_of('.'));
    
    // Create full paths in work directory
    std::string dspPath = getWorkPath(dspfilename);
    std::string cppPath = getWorkPath(baseName + ".cpp");
    std::string errPath = getWorkPath(baseName + ".txt");

    // Store the faust code into file
    std::ofstream outFile(dspPath);
    outFile << srcCode;
    outFile.close();

    // Call the Faust compiler
    std::string command = "faust -o " + cppPath + " " + dspPath + " " +
                          compileOptions + " 2> " + errPath;
    int result = std::system(command.c_str());

    // Check for compilation error
    if (result != 0) {
      auto errData = encodeFile(errPath);
      json resource = *errData;
      return json::array({{{"type", "resource"}, {"resource", resource}}});
    }

    // Read the generated cpp file
    auto fileData = encodeFile(cppPath);

    if (!fileData) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not read generated file"}}}); 
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
