#include "expr_parser.hpp"
#include <iostream>

std::optional<FilterSpec> ExpressionParser::parse_filter(std::string_view expr) noexcept {
  if (expr.empty()) {
    std::cerr << "[Parser] Error: Empty filter expression\n";
    return std::nullopt;
  }

  // Step 1: Find operator (try longest first to match >= before =)
  auto op_result = find_operator(expr);
  if (!op_result) {
    std::cerr << "[Parser] Error: No valid operator found in: " << expr << "\n";
    return std::nullopt;
  }

  auto [op, op_pos] = *op_result;

  // Step 2: Extract field and value
  std::string_view field_str = expr.substr(0, op_pos);
  std::string_view value_str =
      expr.substr(op_pos + (op == FilterOp::Equal || op == FilterOp::GreaterThan || op == FilterOp::LessThan ? 1 : 2));

  // Trim whitespace
  auto trim = [](std::string_view &s) {
    size_t start = s.find_first_not_of(" \t");
    size_t end = s.find_last_not_of(" \t");
    if (start == std::string_view::npos) {
      s = "";
    } else {
      s = s.substr(start, end - start + 1);
    }
  };

  trim(field_str);
  trim(value_str);

  // Step 3: Parse field
  auto field = parse_filter_field(field_str);
  if (!field) {
    std::cerr << "[Parser] Error: Unknown field: " << field_str << "\n";
    return std::nullopt;
  }

  // Step 4: Construct FilterSpec
  return FilterSpec{*field, op, std::string(value_str)};
}

std::optional<SortSpec> ExpressionParser::parse_sort(std::string_view expr) noexcept {
  if (expr.empty()) {
    std::cerr << "[Parser] Error: Empty sort expression\n";
    return std::nullopt;
  }

  // Find space separator
  size_t space_pos = expr.find(' ');

  std::string_view field_str;
  std::string_view direction_str;

  if (space_pos == std::string_view::npos) {
    // No direction specified, default to ascending
    field_str = expr;
    direction_str = "asc";
  } else {
    field_str = expr.substr(0, space_pos);
    direction_str = expr.substr(space_pos + 1);
  }

  // Parse field
  auto field = parse_sort_field(field_str);
  if (!field) {
    std::cerr << "[Parser] Error: Unknown sort field: " << field_str << "\n";
    return std::nullopt;
  }

  // Parse direction
  SortDirection direction = SortDirection::Ascending;
  if (direction_str == "desc" || direction_str == "descending") {
    direction = SortDirection::Descending;
  } else if (direction_str != "asc" && direction_str != "ascending") {
    std::cerr << "[Parser] Warning: Unknown direction '" << direction_str << "', using ascending\n";
  }

  return SortSpec{*field, direction};
}

std::optional<FilterField> ExpressionParser::parse_filter_field(std::string_view field) noexcept {
  if (field == "id")
    return FilterField::Id;
  if (field == "title")
    return FilterField::Title;
  if (field == "status")
    return FilterField::Status;
  if (field == "priority")
    return FilterField::Priority;
  if (field == "created_date")
    return FilterField::CreatedDate;
  if (field == "due_date")
    return FilterField::DueDate;
  if (field == "assignee")
    return FilterField::Assignee;
  if (field == "description")
    return FilterField::Description;
  return std::nullopt;
}

std::optional<SortField> ExpressionParser::parse_sort_field(std::string_view field) noexcept {
  if (field == "id")
    return SortField::Id;
  if (field == "title")
    return SortField::Title;
  if (field == "status")
    return SortField::Status;
  if (field == "priority")
    return SortField::Priority;
  if (field == "created_date")
    return SortField::CreatedDate;
  if (field == "due_date")
    return SortField::DueDate;
  return std::nullopt;
}

std::optional<std::pair<FilterOp, size_t>> ExpressionParser::find_operator(std::string_view expr) noexcept {
  // Try two-character operators first
  size_t pos;

  pos = expr.find(">=");
  if (pos != std::string_view::npos) {
    return std::make_pair(FilterOp::GreaterThanOrEqual, pos);
  }

  pos = expr.find("<=");
  if (pos != std::string_view::npos) {
    return std::make_pair(FilterOp::LessThanOrEqual, pos);
  }

  pos = expr.find("!=");
  if (pos != std::string_view::npos) {
    return std::make_pair(FilterOp::NotEqual, pos);
  }

  // Then single-character operators
  pos = expr.find('=');
  if (pos != std::string_view::npos) {
    return std::make_pair(FilterOp::Equal, pos);
  }

  pos = expr.find('>');
  if (pos != std::string_view::npos) {
    return std::make_pair(FilterOp::GreaterThan, pos);
  }

  pos = expr.find('<');
  if (pos != std::string_view::npos) {
    return std::make_pair(FilterOp::LessThan, pos);
  }

  return std::nullopt;
}
