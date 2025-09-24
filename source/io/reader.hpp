#pragma once
#include "../core/task.hpp"
#include <string>
#include <vector>

/**
 * @brief Abstract interface for task file readers.
 *
 * Implementations parse a file and produce Task objects.
 */
class ITaskReader {
public:
  virtual ~ITaskReader() = default;

  /**
   * @brief Parse tasks from the file at `filepath`.
   * @pre `filepath` is the path to the input file (caller-supplied).
   * @post On success: returns a vector containing parsed Task objects.
   * @post On failure: implementations may return an empty vector or throw
   *       an exception (see @throws). Callers should check return value and/or
   *       catch exceptions depending on the caller policy.
   * @throws std::exception or implementation-specific exceptions on I/O/parse errors.
   *
   * @param filepath Path to the input file.
   * @return Vector of parsed Task objects.
   */
  virtual std::vector<Task> read_tasks(const std::string &filepath) = 0;

  /**
   * @brief Check whether this reader can handle `filepath` (e.g. by extension).
   * @pre `filepath` is a filename or path.
   * @post returns true if this reader recognizes the file format.
   */
  virtual bool can_handle(const std::string &filepath) const = 0;
};
