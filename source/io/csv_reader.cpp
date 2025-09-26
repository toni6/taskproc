#include "io/csv_reader.hpp"
#include "core/task.hpp"
#include <csv.h>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

bool CSVReader::can_handle(const std::string &filepath) const { return filepath.ends_with(".csv"); }

// Pre: `tags_field` is the raw tags string from CSV (no outer quotes), for example "tag1,tag2,tag3".
// Post: returns vector of tag tokens. If `tags_field` is empty returns empty vector.
std::vector<std::string> split_tags(const std::string &tags_field) {
  std::vector<std::string> tags;
  std::string tag;
  std::istringstream iss(tags_field);
  while (std::getline(iss, tag, ',')) {
    tags.emplace_back(std::move(tag));
  }
  return tags;
}

std::vector<Task> CSVReader::read_tasks(const std::string &filepath) {
  // Enable trimming and double-quote escaping (comma separator, double-quote as
  // quote char) Template parameters: column count, trim policy, quote policy
  io::CSVReader<9, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '\"'>> in(filepath);
  in.read_header(io::ignore_extra_column,
                 "id",
                 "title",
                 "status",
                 "priority",
                 "created_date",
                 "description",
                 "assignee",
                 "due_date",
                 "tags");

  std::vector<Task> tasks;

  // Variables for row data
  int id;
  std::string title, status;
  int priority;
  std::string created_date, description, assignee, due_date, tags_field;

  while (in.read_row(id, title, status, priority, created_date, description, assignee, due_date, tags_field)) {
    if (id < 1) {
      std::cerr << "Error while processing task: Invalid ID, must be greater "
                   "than 0\n";
      continue;
    }
    if (title.empty() || status.empty()) {
      std::cerr << "Error while processing task: Invalid title or status\n";
      continue;
    }
    if (priority < 1) {
      priority = 1;
    }

    tasks.emplace_back(
        id, title, status, priority, created_date, description, assignee, due_date, split_tags(tags_field));
  }
  return tasks;
}
