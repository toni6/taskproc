#pragma once
#include "../core/task.hpp"
#include "reader.hpp"
#include <string>
#include <vector>

/**
 * @brief CSV reader implementation using fast-cpp-csv-parser.
 *
 * Format specifics:
 * - Expects RFC-style CSV with commas and double-quote escaping.
 * - Expected header columns (in the CSV): id,title,status,priority,created_date,
 *   description,assignee,due_date,tags
 * - `tags` is returned as a single field; callers should expect a comma-
 *   separated tag string in that field (the reader does not split tags for you).
 *
 * @copydoc ITaskReader::readTasks
 *
 * Error and format-specific behavior:
 * - @throws io::error::can_not_open_file if the file cannot be opened.
 * - Malformed rows are typically skipped; individual row parse errors do not
 *   necessarily cause the entire read to fail (see implementation for details).
 * - @note Thread-safety: caller should assume this is not thread-safe unless
 *   otherwise documented.
 */
class CSVReader : public ITaskReader {
public:
  bool canHandle(const std::string &filepath) const override;
  std::vector<Task> readTasks(const std::string &filepath) override;
};
