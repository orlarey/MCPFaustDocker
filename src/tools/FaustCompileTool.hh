#pragma once

#include "mcpTool.hh"

class FaustCompileTool : public McpTool {
public:
  FaustCompileTool();
  std::string name() const override;
  std::string describe() const override;
  json call(const std::string &args) override;
};