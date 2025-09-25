#include "io/json_reader.hpp"
#include "core/task.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <vector>

using json = nlohmann::json;

bool JSONReader::can_handle(const std::string &filepath) const { return filepath.ends_with(".json"); }

std::vector<Task> JSONReader::read_tasks(const std::string &filepath) {
  std::ifstream file(filepath);
  json tasks_json = json::parse(file);

  std::vector<Task> tasks;

  for (const auto &task_json : tasks_json) {
    int id = task_json.value("id", 0);
    if (id < 1) {
      std::cerr << "Error while processing task: Invalid ID, must be greater "
                   "than 0\n";
      continue;
    }
    std::string title = task_json.value("title", "");
    std::string status = task_json.value("status", "");
    if (title.empty() || status.empty()) {
      std::cerr << "Error while processing task: Invalid title or status\n";
      continue;
    }
    int priority = task_json.value("priority", 1);
    std::string created_date = task_json.value("created_date", "");
    std::string description = task_json.value("description", "");
    std::string assignee = task_json.value("assignee", "");
    std::string due_date = task_json.value("due_date", "");
    std::vector<std::string> tags;
    if (task_json.contains("tags") && task_json.at("tags").is_array()) {
      for (const auto &tag : task_json["tags"]) {
        tags.emplace_back(tag.get<std::string>());
      }
    }

    tasks.emplace_back(id,
                       std::move(title),
                       std::move(status),
                       priority,
                       std::move(created_date),
                       std::move(description),
                       std::move(assignee),
                       std::move(due_date),
                       std::move(tags));
  }

  return tasks;
}
