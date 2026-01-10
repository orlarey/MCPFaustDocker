########################################################################
########################################################################
#
#       MCP Faust Server - Frontend Architecture
#       (Uses external Faust Docker image)
#
########################################################################
########################################################################

ARG BASE_IMAGE=alpine:20251224

########################################################################
# Stage 1: BUILD - Compilation du serveur MCP
########################################################################
FROM ${BASE_IMAGE} AS builder

RUN apk add --no-cache \
    gcc \
    g++ \
    musl-dev \
    make

WORKDIR /build
COPY src/ ./src/

# Compilation du serveur MCP
RUN g++ -std=c++17 \
    -Isrc -Isrc/tools \
    src/mcpFaustServer.cpp \
    src/tools/*.cpp \
    -o mcpFaustServer

########################################################################
# Stage 2: RUNTIME - Image finale légère
########################################################################
FROM ${BASE_IMAGE}

# Installation des dépendances runtime
RUN apk add --no-cache \
    libstdc++ \
    docker-cli

# Copie du binaire compilé depuis le stage builder
COPY --from=builder /build/mcpFaustServer /usr/local/bin/

# Création du répertoire de travail
RUN mkdir -p /tmp/faust-mcp
WORKDIR /tmp/faust-mcp

ENTRYPOINT ["/usr/local/bin/mcpFaustServer"]


# Configuration pour Claude Desktop:
# "mcpServers": {
#     "faust": {
#         "command": "docker",
#         "args": [
#             "run", "-i", "--rm",
#             "-v", "/var/run/docker.sock:/var/run/docker.sock",
#             "-v", "/tmp/faust-shared:/tmp/faust-mcp",
#             "mcp-faust-server"
#         ]
#     }
# }
