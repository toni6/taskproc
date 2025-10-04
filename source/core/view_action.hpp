#include <optional>
#include <string>
#include <string_view>

/**
 * @brief Domain-level view operation to be recorded and replayed.
 *
 * @note Decoupled from the CLI. The CLI maps argv to DataManager methods,
 *       and DataManager records successful operations as ViewAction.
 */
enum class ViewOpType {
  Load,         ///< initial load from filepath (optional to persist)
  Filter,       ///< apply filter expression
  Sort,         ///< apply sort expression
  ResetFilters, ///< reset/clear filters
  FindByTag     ///< filter by tag
};

/**
 * @brief A single recorded view action with its payload/argument.
 *
 * Examples:
 *  - {ViewOpType::Filter, "priority<=3"}
 *  - {ViewOpType::Sort, "due_date desc"}
 */
struct ViewAction {
  ViewOpType type;
  std::string payload;
};

/**
 * @brief Convert a ViewOpType to a stable string for persistence/logging.
 * @pre none
 * @post Returns a lowercase identifier (e.g., "filter", "sort").
 */
inline std::string to_string(ViewOpType t) noexcept {
  switch (t) {
  case ViewOpType::Load:
    return "load";
  case ViewOpType::Filter:
    return "filter";
  case ViewOpType::Sort:
    return "sort";
  case ViewOpType::ResetFilters:
    return "reset-filters";
  case ViewOpType::FindByTag:
    return "find-by-tag";
  }
  return "unknown";
}

/**
 * @brief Parse a string into a ViewOpType.
 * @pre none
 * @post Returns a matching enum if recognized, std::nullopt otherwise.
 */
inline std::optional<ViewOpType> view_op_type_from_string(std::string_view s) noexcept {
  if (s == "load")
    return ViewOpType::Load;
  if (s == "filter")
    return ViewOpType::Filter;
  if (s == "sort")
    return ViewOpType::Sort;
  if (s == "reset-filters")
    return ViewOpType::ResetFilters;
  if (s == "find-by-tag")
    return ViewOpType::FindByTag;
  return std::nullopt;
}
