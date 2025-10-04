#include "io/csv_reader.hpp"
#include "core/task.hpp"
#include <csv.h>
#include <iostream>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

namespace {
// Pre: `tags_field` is the raw tags string from CSV (no outer quotes), for example "tag1,tag2,tag3".
// Post: returns vector of tag tokens. If `tags_field` is empty returns empty vector.
std::vector<std::string> split_tags(std::string tags_field) {
  std::vector<std::string> tags;
  std::string tag;
  std::istringstream iss(tags_field);
  while (std::getline(iss, tag, ',')) {
    tags.push_back(std::move(tag));
  }
  return tags;
}
} // anonymous namespace

bool CSVReader::can_handle(std::string_view filepath) const { return filepath.ends_with(".csv"); }

std::vector<Task> CSVReader::read_tasks(std::string_view filepath) {
  // Enable trimming and double-quote escaping (comma separator, double-quote as
  // quote char) Template parameters: column count, trim policy, quote policy
  constexpr int CSV_COLUMNS = 9;
  io::CSVReader<CSV_COLUMNS, io::trim_chars<' ', '\t'>, io::double_quote_escape<',', '\"'>> in{std::string(filepath)};
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

    tasks.emplace_back(id,
                       std::move(title),
                       std::move(status),
                       priority,
                       std::move(created_date),
                       std::move(description),
                       std::move(assignee),
                       std::move(due_date),
                       split_tags(std::move(tags_field)));
  }
  return tasks;
}
