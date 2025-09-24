#include "io/json_reader.hpp"
#include "core/task.hpp"
#include <fstream>
#include <sstream>
#include <vector>

bool JSONReader::can_handle(const std::string &filepath) const { return filepath.ends_with(".json"); }

// TODO: Replace with JSON parsing logic
std::vector<Task> JSONReader::read_tasks(const std::string &filepath) {
  std::ifstream file(filepath);
  std::string line;
  std::vector<Task> tasks;

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string id, title;

    if (!(iss >> id >> title)) {
      continue;
    }

    int id_int = std::stoi(id);
    tasks.emplace_back(id_int, title);
  }

  return tasks;
}
