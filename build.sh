#!/bin/bash

########################################################################
# Build script for MCP Faust Server (Frontend Architecture)
#
# This version uses an external Faust Docker image instead of
# embedding the Faust compiler in the MCP server image.
########################################################################

echo "Building MCP Faust Server (frontend architecture)..."
docker build -t mcpfaustdocker .

echo ""
echo "Build complete!"
echo ""
echo "Configuration for Claude Desktop:"
echo ""
echo '{
  "mcpServers": {
    "faust": {
      "command": "docker",
      "args": [
        "run", "-i", "--rm",
        "-v", "/var/run/docker.sock:/var/run/docker.sock",
        "-v", "/tmp/faust-shared:/tmp/faust-mcp",
        "mcpfaustdocker"
      ]
    }
  }
}'
echo ""
