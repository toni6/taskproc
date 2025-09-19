#pragma once
#include "../core/task.hpp"
#include <string>
#include <vector>

class ITaskReader {
public:
  virtual ~ITaskReader() = default;
  virtual std::vector<Task> readTasks(const std::string &filepath) = 0;
  virtual bool canHandle(const std::string &filepath) const = 0;
};
