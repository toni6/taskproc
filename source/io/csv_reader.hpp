#pragma once
#include "../core/task.hpp"
#include "reader.hpp"
#include <string>
#include <vector>

class CSVReader : public ITaskReader {
public:
  bool canHandle(const std::string &filepath) const override;
  std::vector<Task> readTasks(const std::string &filepath) override;
};
