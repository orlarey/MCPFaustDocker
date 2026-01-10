# MCPFaustDocker (Frontend Architecture)

A Model Context Protocol (MCP) server for the Faust audio programming language, providing compilation and analysis tools through a containerized environment.

## Introduction

This project implements an MCP server that exposes Faust compiler functionality to Large Language Models (LLMs) like Claude. The server allows LLMs to compile Faust DSP code, generate block diagrams, and interact with the Faust ecosystem.

[Faust](https://faust.grame.fr) (Functional Audio Stream) is a functional programming language specifically designed for real-time signal processing and synthesis. Through this MCP server, LLMs can:
- Compile Faust DSP code to various target languages (C++, WebAssembly, etc.)
- Generate visual representations of signal processing graphs
- Query Faust compiler version and capabilities

### Architecture

**Frontend Architecture**: This branch uses an **external Faust Docker image** ([ghcr.io/orlarey/faustdocker](https://github.com/orlarey/faustdocker)) instead of embedding the Faust compiler directly in the MCP server container. This provides:

- ✅ **Lightweight images**: MCP server ~50MB (Alpine-based) vs ~1.2GB (gcc + Faust)
- ✅ **Separation of concerns**: MCP logic separate from Faust compiler
- ✅ **Easy updates**: Update Faust independently with `docker pull`
- ✅ **Consistent tooling**: Same Faust image for CLI and MCP usage
- ✅ **Multi-stage builds**: Both images use Alpine Linux for minimal size

This is built as an extension of the [MCPHelloWorldDocker](https://github.com/orlarey/MCPHelloWorldDocker) project. For detailed information about the MCP protocol implementation, JSON-RPC communication, and architectural design, please refer to the [MCPHelloWorldDocker README](https://github.com/orlarey/MCPHelloWorldDocker/blob/main/README.md).

## Available Tools

The server currently provides four tools for interacting with Faust:

### FaustVersionTool
Returns the version of the Faust compiler installed in the container. This tool helps verify the compilation environment and ensures compatibility with specific Faust features.

**Parameters:** None

### FaustCompileTool
Compiles Faust DSP source code to C++. This tool:
- Accepts Faust source code as input
- Supports optional compilation flags
- Returns the generated C++ code as a resource
- Handles compilation errors gracefully

**Parameters:**
- `value` (required, string): The Faust DSP source code to compile
- `options` (optional, string): Additional compilation flags for the Faust compiler

### FaustSVGTool
Generates SVG block diagrams from Faust code, providing visual representations of the signal processing graph. This helps understand the data flow and structure of DSP algorithms.

**Parameters:**
- `value` (required, string): The Faust DSP source code to visualize

### FaustHelpTool
Returns comprehensive help information about the Faust compiler, including all available compilation options, flags, and architectures. This is essential for understanding advanced compilation features.

**Parameters:** None

## Project Structure

```
MCPFaustDocker/
├── src/
│   ├── mcpFaustServer.cpp     # Main server application
│   ├── mcpServer.hh           # MCP server implementation
│   ├── json.hpp               # JSON parsing library
│   └── tools/
│       ├── mcpTool.hh         # Base class for tools
│       ├── FaustVersionTool.cpp/hh
│       ├── FaustCompileTool.cpp/hh
│       ├── FaustSVGTool.cpp/hh
│       ├── FaustHelpTool.cpp/hh
│       └── utils.cpp/hh       # Helper functions
├── Dockerfile
├── build.sh
└── README.md
```

## Building and Running

### Prerequisites
- Docker installed on your system
- An MCP client (e.g., Claude Desktop)

### Building the Docker Image

Run the provided build script:

```bash
./build.sh
```

Or build manually:

```bash
docker build -t mcp-faust-server .
```

The Dockerfile uses a **multi-stage build** pattern:
- **Stage 1 (Builder)**: Alpine Linux with gcc/g++ to compile the MCP server
- **Stage 2 (Runtime)**: Minimal Alpine Linux with only `libstdc++` and `docker-cli`
- Same base image (`alpine:20251224`) as the Faust Docker image for consistency

### MCP Client Configuration

To use this server with an MCP client like Claude Desktop, add the following configuration to your MCP settings:

```json
{
  "mcpServers": {
    "faust": {
      "command": "docker",
      "args": [
        "run", "-i", "--rm",
        "-v", "/var/run/docker.sock:/var/run/docker.sock",
        "-v", "/tmp/faust-shared:/tmp/faust-mcp",
        "mcp-faust-server"
      ]
    }
  }
}
```

**Important flags:**
- `-i`: Interactive mode for stdin/stdout communication
- `--rm`: Automatically removes the container when it exits
- `-v /var/run/docker.sock:/var/run/docker.sock`: Grants access to Docker daemon (required for calling external Faust container)
- `-v /tmp/faust-shared:/tmp/faust-mcp`: Shared volume for file exchange between MCP and Faust containers

## Usage Examples

Once configured, you can interact with the Faust compiler through your MCP client:

### Example 1: Check Faust Version
Ask your LLM to "Check the Faust compiler version" to verify the installation.

### Example 2: Compile a Simple Oscillator
```faust
import("stdfaust.lib");
freq = 440;
process = os.osc(freq);
```
Request: "Compile this Faust code to C++"

### Example 3: Create a Filter
```faust
import("stdfaust.lib");
cutoff = hslider("cutoff", 1000, 20, 20000, 1);
process = fi.lowpass(2, cutoff);
```
Request: "Compile this low-pass filter"

### Example 4: Generate a Block Diagram
```faust
process = _ : *(0.5) : +(0.1);
```
Request: "Generate an SVG diagram for this Faust code"

## Technical Details

This server implements the MCP protocol from scratch using:
- Standard C++ with C++17 features
- The `nlohmann/json` library for JSON parsing
- Docker-in-Docker pattern to call external Faust compiler

### How It Works

1. **MCP Server Container** runs with access to the Docker daemon via mounted socket
2. **Tool calls** write DSP code to `/tmp/faust-mcp/` directory
3. **`runFaustDocker()`** launches `ghcr.io/orlarey/faustdocker:main` with:
   - Mounted work directory: `-v /tmp/faust-mcp:/tmp`
   - Faust compilation arguments
4. **Generated files** are written back to the shared directory
5. **MCP Server** reads results and returns them to the LLM

Communication occurs through JSON-RPC 2.0 messages over stdio, following the MCP specification. Each tool inherits from the `McpTool` base class and implements:
- `name()`: Returns the tool identifier
- `describe()`: Provides the tool's JSON schema
- `call()`: Executes the tool with given arguments

## Future Enhancements

This is an early version focusing on core compilation features. Potential improvements include:
- Additional output formats (WebAssembly, JUCE, VST, etc.)
- Real-time compilation with error diagnostics
- Library management and import resolution
- Integration with Faust IDE features
- Audio file generation and processing

## References

- [Faust Programming Language](https://faust.grame.fr)
- [Faust Docker Image](https://github.com/orlarey/faustdocker) - External Faust compiler used by this server
- [MCPHelloWorldDocker](https://github.com/orlarey/MCPHelloWorldDocker) - Base implementation and MCP protocol details
- [Model Context Protocol Specification](https://modelcontextprotocol.io)
- [Faust Documentation](https://faustdoc.grame.fr)

## License

[Same license as the original MCPHelloWorldDocker project]