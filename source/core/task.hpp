#pragma once
#include <optional>
#include <string>
#include <vector>

/// Represents a task with various attributes.
struct Task {
  int id;
  std::string title;
  std::string status;       // "todo", "in-progress", "done"
  int priority;             // 1-5 (5 = highest)
  std::string created_date; // ISO 8601 format
  std::optional<std::string> description;
  std::optional<std::string> assignee;
  std::optional<std::string> due_date; // ISO 8601 format
  std::vector<std::string> tags;

  Task(int id_,
       std::string title_,
       std::string status_ = "todo",
       int priority_ = 1,
       std::string created_date_ = "",
       std::optional<std::string> description_ = std::nullopt,
       std::optional<std::string> assignee_ = std::nullopt,
       std::optional<std::string> due_date_ = std::nullopt,
       std::vector<std::string> tags_ = {}) noexcept :
      id(id_),
      title(std::move(title_)),
      status(std::move(status_)),
      priority(priority_),
      created_date(std::move(created_date_)),
      description(std::move(description_)),
      assignee(std::move(assignee_)),
      due_date(std::move(due_date_)),
      tags(std::move(tags_)) {}
};

/**
 * @brief Stream insertion operator for Task (one-line summary format).
 *
 * @pre `os` is a valid output stream.
 * @post Writes a compact task representation to the stream.
 * @throws Whatever the underlying stream operations throw.
 *
 * Format: "ID: 1 | Title: Fix bug | Status: todo | Priority: 3"
 *
 * @param os Output stream.
 * @param task Task to output.
 * @return Reference to the stream (for chaining).
 */
std::ostream &operator<<(std::ostream &os, const Task &task);
