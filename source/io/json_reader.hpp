#pragma once
#include "../core/task.hpp"
#include "reader.hpp"
#include <string>
#include <vector>

/**
 * @brief JSONReader is a class that reads tasks from a JSON file.
 *
 * TODO Implement the readTasks method to read tasks from a JSON file with an appropriate JSON parser.
 */
class JSONReader : public ITaskReader {
public:
  bool canHandle(const std::string &filepath) const override;
  std::vector<Task> readTasks(const std::string &filepath) override;
};
