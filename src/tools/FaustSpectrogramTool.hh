#pragma once

#include "mcpTool.hh"

class FaustSpectrogramTool : public McpTool {
public:
  FaustSpectrogramTool();
  std::string name() const override;
  std::string describe() const override;
  json call(const std::string &args) override;
};
