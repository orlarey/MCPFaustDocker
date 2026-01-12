#pragma once

#include <string>
#include <cstdlib>

// Configuration for Faust MCP serveur

// Work directory management (internal container path)
const std::string WORK_DIR = "/tmp/faust-mcp";

// Faust Docker image configuration
const std::string FAUST_DOCKER_IMAGE = "ghcr.io/orlarey/faustdocker:main";

// Host shared directory (for Docker-in-Docker mounting)
const std::string HOST_SHARED_DIR = "/tmp/faust-shared";
