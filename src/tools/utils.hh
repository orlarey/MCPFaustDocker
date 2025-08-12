#pragma once

#include <fstream>
#include <optional>
#include <string>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>

#include "json.hpp"

using json = nlohmann::json;

// Work directory management
const std::string WORK_DIR = "/tmp/faust-mcp";
bool ensureWorkDir();
std::string getWorkPath(const std::string &filename);

// Base64 encoding function
std::string base64_encode(const std::vector<unsigned char> &data);

// Encode file to base64 and return JSON
std::optional<json> encodeFile(const std::string &filepath);
