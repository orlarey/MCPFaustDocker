#include "FaustSpectrogramTool.hh"
#include "utils.hh"
#include <cstdlib>
#include <sstream>

// Constructor
FaustSpectrogramTool::FaustSpectrogramTool() {
  // Initialize tool-specific data here
}

// Returns the tool name for MCP registration
std::string FaustSpectrogramTool::name() const {
  return "FaustSpectrogramTool";
}

// Returns the tool description and schema for MCP
std::string FaustSpectrogramTool::describe() const {
  // Build tool description using JSON object
  json description = {
      {"name", name()},
      {"description",
       "Generates mel-scale spectrogram PNG from Faust DSP code. The DSP must "
       "expose three parameters: 'gate' (button), 'freq' (frequency), and "
       "'gain' (amplitude)."},
      {"inputSchema",
       {{"type", "object"},
        {"properties",
         {{"value",
           {{"type", "string"},
            {"description", "Faust DSP source code (must expose: gate, freq, "
                            "gain parameters)"}}},
          {"duration",
           {{"type", "number"},
            {"description", "Total duration in seconds"},
            {"default", 2.0}}},
          {"gate_duration",
           {{"type", "number"},
            {"description", "Gate=1 duration in seconds (from start)"},
            {"default", 0.5}}},
          {"frequency",
           {{"type", "number"},
            {"description", "Frequency in Hz"},
            {"default", 440.0}}},
          {"gain",
           {{"type", "number"},
            {"description", "Gain value (0.0 to 1.0)"},
            {"default", 0.8}}},
          {"sample_rate",
           {{"type", "number"},
            {"description", "Sample rate in Hz"},
            {"default", 44100}}},
          {"fft_size",
           {{"type", "number"},
            {"description", "FFT size (power of 2)"},
            {"default", 2048}}},
          {"hop_size",
           {{"type", "number"},
            {"description", "Hop size in samples"},
            {"default", 512}}},
          {"mel_bands",
           {{"type", "number"},
            {"description", "Number of mel bands"},
            {"default", 128}}},
          {"colormap",
           {{"type", "string"},
            {"description", "Colormap: viridis, magma, hot, gray"},
            {"default", "hot"}}},
          {"use_db",
           {{"type", "boolean"},
            {"description", "Display in decibels"},
            {"default", false}}}}},
        {"required", json::array({"value"})}}}};

  return description.dump();
}

// Generates spectrogram PNG from Faust DSP code
json FaustSpectrogramTool::call(const std::string &args) {
  try {
    // Ensure work directory exists
    if (!ensureWorkDir()) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not create work directory"}}});
    }

    // Parse the JSON arguments
    json arguments = json::parse(args);

    // Extract parameters
    std::string srcCode = arguments.value("value", "process = _;");
    double duration = arguments.value("duration", 2.0);
    double gate_duration = arguments.value("gate_duration", 0.5);
    double frequency = arguments.value("frequency", 440.0);
    double gain = arguments.value("gain", 0.8);
    int sample_rate = arguments.value("sample_rate", 44100);
    int fft_size = arguments.value("fft_size", 2048);
    int hop_size = arguments.value("hop_size", 512);
    int mel_bands = arguments.value("mel_bands", 128);
    std::string colormap = arguments.value("colormap", "hot");
    bool use_db = arguments.value("use_db", false);

    // Create paths in work directory
    std::string dspPath = getWorkPath("spectrogram_source.dsp");
    std::string archPath = getWorkPath("spectrogram.cpp");
    std::string cppPath = getWorkPath("spectrogram_source.cpp");
    std::string exePath = getWorkPath("spectrogram_exe");
    std::string pngPath = getWorkPath("spectrogram.png");
    std::string errPath = getWorkPath("spectrogram_error.txt");

    // Store the Faust code into file
    std::ofstream outFile(dspPath);
    outFile << srcCode;
    outFile.close();

    // Copy spectrogram.cpp architecture to work directory (shared volume)
    // It needs to be in the shared directory for faustdocker to access it
    std::ifstream archIn("/usr/local/share/faust/spectrogram.cpp", std::ios::binary);
    if (!archIn) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not read spectrogram.cpp architecture"}}});
    }
    std::ofstream archOut(archPath, std::ios::binary);
    archOut << archIn.rdbuf();
    archIn.close();
    archOut.close();

    // Step 1: Compile DSP to C++ using spectrogram.cpp architecture via Docker
    // All files must be in work directory (mounted as /tmp in faustdocker)
    auto result = runFaustDocker("-a spectrogram.cpp -o spectrogram_source.cpp spectrogram_source.dsp");

    if (result.exitCode != 0) {
      // Write stderr to error file
      std::ofstream errFile(errPath);
      errFile << "Faust compilation error:\n" << result.errorOutput;
      errFile.close();

      return json::array(
          {{{"type", "text"},
            {"text", "Error: Faust compilation failed: " + result.errorOutput}}});
    }

    // Step 2: Compile C++ to executable using g++ in the MCP container
    std::ostringstream compileCmd;
    compileCmd << "g++ " << cppPath << " -o " << exePath
               << " -std=c++11 -O3"
               << " -I/usr/local/include"
               << " -L/usr/local/lib"
               << " -lfftw3f -lpng -lm"
               << " 2>&1";

    FILE *pipe = popen(compileCmd.str().c_str(), "r");
    if (!pipe) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not execute g++ compiler"}}});
    }

    char buffer[256];
    std::string compileOutput;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      compileOutput += buffer;
    }
    int compileStatus = pclose(pipe);

    if (compileStatus != 0) {
      std::ofstream errFile(errPath);
      errFile << "C++ compilation error:\n" << compileOutput;
      errFile.close();

      return json::array(
          {{{"type", "text"},
            {"text", "Error: C++ compilation failed: " + compileOutput}}});
    }

    // Step 3: Execute the spectrogram generator
    std::ostringstream execCmd;
    execCmd << exePath << " " << duration << " " << gate_duration << " "
            << frequency << " " << gain << " -sr " << sample_rate << " -fft "
            << fft_size << " -hop " << hop_size << " -mel " << mel_bands
            << " -cmap " << colormap << " -o " << pngPath;

    if (use_db) {
      execCmd << " -db";
    }

    execCmd << " 2>&1";

    pipe = popen(execCmd.str().c_str(), "r");
    if (!pipe) {
      return json::array(
          {{{"type", "text"},
            {"text", "Error: Could not execute spectrogram generator"}}});
    }

    std::string execOutput;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      execOutput += buffer;
    }
    int execStatus = pclose(pipe);

    if (execStatus != 0) {
      std::ofstream errFile(errPath);
      errFile << "Spectrogram generation error:\n" << execOutput;
      errFile.close();

      return json::array(
          {{{"type", "text"},
            {"text", "Error: Spectrogram generation failed: " + execOutput}}});
    }

    // Step 4: Read the generated PNG file
    auto fileData = encodeFile(pngPath);

    if (!fileData) {
      return json::array(
          {{{"type", "text"}, {"text", "Error: Could not read PNG file at: " + pngPath}}});
    }

    // Extract base64 data and mimeType from encodeFile result
    json imageData = *fileData;
    std::string base64Data = imageData.value("data", "");
    std::string mimeType = imageData.value("mimeType", "image/png");

    if (base64Data.empty()) {
      return json::array(
          {{{"type", "text"}, {"text", "Error: PNG file is empty or could not be encoded"}}});
    }

    // Return as MCP content array with only the image
    return json::array({
      {{"type", "image"}, {"data", base64Data}, {"mimeType", mimeType}}
    });

  } catch (const json::parse_error &e) {
    // Handle parse error
    return json::array(
        {{{"type", "text"}, {"text", "Error: Invalid arguments"}}});
  } catch (const std::exception &e) {
    return json::array(
        {{{"type", "text"}, {"text", std::string("Error: ") + e.what()}}});
  }
}
