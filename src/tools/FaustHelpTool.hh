#pragma once

#include "mcpTool.hh"

class FaustHelpTool : public McpTool {
public:
  FaustHelpTool();
  std::string name() const override;
  std::string describe() const override;
  json call(const std::string &args) override;
};