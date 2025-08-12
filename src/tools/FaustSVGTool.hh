#pragma once

#include "mcpTool.hh"

class FaustSVGTool : public McpTool {
public:
  FaustSVGTool();
  std::string name() const override;
  std::string describe() const override;
  json call(const std::string &args) override;
};