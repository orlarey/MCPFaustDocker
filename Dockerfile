# Base image with C++ compilation tools
FROM gcc:latest

RUN apt update && apt install -y cmake

# Clone, compile and install faust
WORKDIR /
RUN git clone https://github.com/grame-cncm/faust.git
WORKDIR /faust
RUN git fetch && git checkout 66b3351
RUN make -C /faust/build cmake
RUN make -C /faust/build 
RUN make -C /faust/build install


# Compiling the MCP Faust Server

WORKDIR /app
COPY src/ ./src/
RUN g++ -std=c++17 -Isrc -Isrc/tools  src/mcpFaustServer.cpp src/tools/*.cpp -o mcpFaustServer

# Set entry point
ENTRYPOINT ["/app/mcpFaustServer"]


# "mcpServers": {
#     "mcpFaustServer": {
#         "command": "docker",
#         "args": ["run", "-i", "mcpFaustServer"]
#     }
# }
