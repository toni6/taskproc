#pragma once
#include "../core/task.hpp"
#include "reader.hpp"
#include <string_view>
#include <vector>

/**
 * @brief JSON reader implementation using nlohmann::json.
 *
 * Format specifics:
 * - Expects a top-level JSON array of task objects.
 * - Each task object should contain at least the fields:
 *     - "id" (integer, required)
 *     - "title" (string, required)
 *     - "status" (string, required)
 * - Optional fields:
 *     - "priority" (integer, default: 1)
 *     - "created_date" (string, ISO 8601)
 *     - "description" (string)
 *     - "assignee" (string)
 *     - "due_date" (string, ISO 8601)
 *     - "tags" (array of strings)
 *
 * Example JSON entry:
 * {
 *   "id": 1,
 *   "title": "Fix login page bug",
 *   "status": "todo",
 *   "priority": 5,
 *   "created_date": "2024-01-15",
 *   "description": "Users cannot log in with special characters",
 *   "assignee": "john.doe",
 *   "due_date": "2024-01-20",
 *   "tags": ["bug", "urgent"]
 * }
 *
 * @copydoc ITaskReader::read_tasks
 *
 * Error and format-specific behavior:
 * - The reader will typically skip malformed task objects (missing required fields
 *   or invalid id) and continue parsing the rest of the array.
 * - Parsing or I/O failures may throw exceptions (e.g. from nlohmann::json or std::ifstream).
 * - @note Thread-safety: caller should assume this is not thread-safe unless
 *   otherwise documented.
 */
class JSONReader : public ITaskReader {
public:
  bool can_handle(std::string_view filepath) const override;
  std::vector<Task> read_tasks(std::string_view filepath) override;
};
