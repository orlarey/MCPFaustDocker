# MCPFaustDocker

A Model Context Protocol (MCP) server for the Faust audio programming language, providing compilation and analysis tools through a containerized environment.

## Introduction

This project implements an MCP server that exposes Faust compiler functionality to Large Language Models (LLMs) like Claude. The server allows LLMs to compile Faust DSP code, generate block diagrams, and interact with the Faust ecosystem.

[Faust](https://faust.grame.fr) (Functional Audio Stream) is a functional programming language specifically designed for real-time signal processing and synthesis. Through this MCP server, LLMs can:
- Compile Faust DSP code to various target languages (C++, WebAssembly, etc.)
- Generate visual representations of signal processing graphs
- Query Faust compiler version and capabilities

This is an early version of the server, built as an extension of the [MCPHelloWorldDocker](https://github.com/orlarey/MCPHelloWorldDocker) project. For detailed information about the MCP protocol implementation, JSON-RPC communication, and architectural design, please refer to the [MCPHelloWorldDocker README](https://github.com/orlarey/MCPHelloWorldDocker/blob/main/README.md).

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

The Dockerfile:
- Uses `gcc:latest` as the base image
- Installs the Faust compiler
- Compiles the MCP server with all tool implementations
- Sets up the containerized environment

### MCP Client Configuration

To use this server with an MCP client like Claude Desktop, add the following configuration to your MCP settings:

```json
{
  "mcpServers": {
    "faust": {
      "command": "docker",
      "args": ["run", "-i", "--rm", "mcp-faust-server"]
    }
  }
}
```

The `-i` flag enables interactive mode for stdin/stdout communication. The `--rm` flag automatically removes the container when it exits.

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
- Direct system calls to the Faust compiler

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
- [MCPHelloWorldDocker](https://github.com/orlarey/MCPHelloWorldDocker) - Base implementation and MCP protocol details
- [Model Context Protocol Specification](https://modelcontextprotocol.io)
- [Faust Documentation](https://faustdoc.grame.fr)

## License

[Same license as the original MCPHelloWorldDocker project]