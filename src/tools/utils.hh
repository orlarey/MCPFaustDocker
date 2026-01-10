#pragma once

#include <fstream>
#include <optional>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>

#include "json.hpp"
#include "config.hh"

using json = nlohmann::json;

// Work directory management
bool ensureWorkDir();
std::string getWorkPath(const std::string &filename);

// Base64 encoding function
std::string base64_encode(const std::vector<unsigned char> &data);

// Encode file to base64 and return JSON
std::optional<json> encodeFile(const std::string &filepath);

// Docker Faust integration
struct FaustDockerResult {
  int exitCode;
  std::string output;      // stdout
  std::string errorOutput; // stderr
};

FaustDockerResult runFaustDocker(
    const std::string& faustArgs,
    const std::string& workDir = WORK_DIR
);
