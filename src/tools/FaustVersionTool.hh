#pragma once

#include "mcpTool.hh"

class FaustVersionTool : public McpTool {
public:
  FaustVersionTool();
  std::string name() const override;
  std::string describe() const override;
  json call(const std::string &args) override;
};