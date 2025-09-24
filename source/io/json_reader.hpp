#pragma once
#include "../core/task.hpp"
#include "reader.hpp"
#include <string>
#include <vector>

/**
 * @brief JSONReader is a class that reads tasks from a JSON file.
 *
 * TODO Implement the read_tasks method to read tasks from a JSON file with an appropriate JSON parser.
 */
class JSONReader : public ITaskReader {
public:
  bool can_handle(const std::string &filepath) const override;
  std::vector<Task> read_tasks(const std::string &filepath) override;
};
