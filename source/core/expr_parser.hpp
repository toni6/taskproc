#pragma once
#include "database.hpp"
#include <optional>
#include <string_view>

/**
 * @brief Parser for filter and sort expressions.
 *
 * Converts user-friendly string expressions into typed specifications
 * that the Database can execute.
 */
class ExpressionParser {
public:
  /**
   * @brief Parse a filter expression string into FilterSpec.
   *
   * Supported formats:
   * - "field=value"       → Equal
   * - "field!=value"      → NotEqual
   * - "field>value"       → GreaterThan
   * - "field>=value"      → GreaterThanOrEqual
   * - "field<value"       → LessThan
   * - "field<=value"      → LessThanOrEqual
   *
   * Supported fields:
   * - id, title, status, priority, created_date, due_date, assignee, description
   *
   * Examples:
   * - "priority>=3"
   * - "status=todo"
   * - "created_date>2024-01-01"
   *
   * @pre `expr` is a non-empty filter expression string.
   * @post Returns FilterSpec if parse succeeds, std::nullopt if invalid.
   * @throws none (returns nullopt on error).
   *
   * @param expr The filter expression to parse.
   * @return Optional FilterSpec (nullopt if parse fails).
   */
  static std::optional<FilterSpec> parse_filter(std::string_view expr) noexcept;

  /**
   * @brief Parse a sort expression string into SortSpec.
   *
   * Supported formats:
   * - "field"             → Ascending (default)
   * - "field asc"         → Ascending (explicit)
   * - "field desc"        → Descending
   *
   * Supported fields:
   * - id, title, status, priority, created_date, due_date
   *
   * Examples:
   * - "priority desc"
   * - "due_date"
   * - "created_date asc"
   *
   * @pre `expr` is a non-empty sort expression string.
   * @post Returns SortSpec if parse succeeds, std::nullopt if invalid.
   * @throws none (returns nullopt on error).
   *
   * @param expr The sort expression to parse.
   * @return Optional SortSpec (nullopt if parse fails).
   */
  static std::optional<SortSpec> parse_sort(std::string_view expr) noexcept;

private:
  /// Parse field name to FilterField enum
  static std::optional<FilterField> parse_filter_field(std::string_view field) noexcept;

  /// Parse field name to SortField enum
  static std::optional<SortField> parse_sort_field(std::string_view field) noexcept;

  /// Detect operator in expression and return {op, position}
  static std::optional<std::pair<FilterOp, size_t>> find_operator(std::string_view expr) noexcept;
};
