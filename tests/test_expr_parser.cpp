#include "core/expr_parser.hpp"
#include <catch2/catch_test_macros.hpp>

// ============================================================================
// Filter Parsing Tests
// ============================================================================

TEST_CASE("ExpressionParser filter parsing", "[core][expr_parser]") {
  SECTION("Parse filter with equal operator") {
    auto result = ExpressionParser::parse_filter("status=todo");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::Status);
    REQUIRE(result->op == FilterOp::Equal);
    REQUIRE(result->value == "todo");
  }

  SECTION("Parse filter with not equal operator") {
    auto result = ExpressionParser::parse_filter("status!=done");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::Status);
    REQUIRE(result->op == FilterOp::NotEqual);
    REQUIRE(result->value == "done");
  }

  SECTION("Parse filter with greater than or equal") {
    auto result = ExpressionParser::parse_filter("priority>=3");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::Priority);
    REQUIRE(result->op == FilterOp::GreaterThanOrEqual);
    REQUIRE(result->value == "3");
  }

  SECTION("Parse filter with less than or equal") {
    auto result = ExpressionParser::parse_filter("priority<=2");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::Priority);
    REQUIRE(result->op == FilterOp::LessThanOrEqual);
    REQUIRE(result->value == "2");
  }

  SECTION("Parse filter with greater than") {
    auto result = ExpressionParser::parse_filter("priority>4");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::Priority);
    REQUIRE(result->op == FilterOp::GreaterThan);
    REQUIRE(result->value == "4");
  }

  SECTION("Parse filter with less than") {
    auto result = ExpressionParser::parse_filter("priority<2");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::Priority);
    REQUIRE(result->op == FilterOp::LessThan);
    REQUIRE(result->value == "2");
  }

  SECTION("Parse filter with whitespace") {
    auto result = ExpressionParser::parse_filter("  status = todo  ");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::Status);
    REQUIRE(result->op == FilterOp::Equal);
    REQUIRE(result->value == "todo");
  }

  SECTION("Parse filter with date field") {
    auto result = ExpressionParser::parse_filter("created_date>2024-01-01");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::CreatedDate);
    REQUIRE(result->op == FilterOp::GreaterThan);
    REQUIRE(result->value == "2024-01-01");
  }

  SECTION("Parse filter with all supported fields") {
    REQUIRE(ExpressionParser::parse_filter("id=1").has_value());
    REQUIRE(ExpressionParser::parse_filter("title=test").has_value());
    REQUIRE(ExpressionParser::parse_filter("status=todo").has_value());
    REQUIRE(ExpressionParser::parse_filter("priority=3").has_value());
    REQUIRE(ExpressionParser::parse_filter("created_date=2024-01-01").has_value());
    REQUIRE(ExpressionParser::parse_filter("due_date=2024-12-31").has_value());
    REQUIRE(ExpressionParser::parse_filter("assignee=john").has_value());
    REQUIRE(ExpressionParser::parse_filter("description=test").has_value());
  }

  SECTION("Invalid filter expressions return nullopt") {
    REQUIRE_FALSE(ExpressionParser::parse_filter("").has_value());
    REQUIRE_FALSE(ExpressionParser::parse_filter("invalid_field=value").has_value());
    REQUIRE_FALSE(ExpressionParser::parse_filter("priority").has_value());
    REQUIRE_FALSE(ExpressionParser::parse_filter("no_operator_here").has_value());
  }
}

// ============================================================================
// Sort Parsing Tests
// ============================================================================

TEST_CASE("ExpressionParser sort parsing", "[core][expr_parser]") {
  SECTION("Parse sort with field only (default ascending)") {
    auto result = ExpressionParser::parse_sort("priority");
    REQUIRE(result.has_value());
    REQUIRE(result->field == SortField::Priority);
    REQUIRE(result->direction == SortDirection::Ascending);
  }

  SECTION("Parse sort with explicit ascending") {
    auto result = ExpressionParser::parse_sort("priority asc");
    REQUIRE(result.has_value());
    REQUIRE(result->field == SortField::Priority);
    REQUIRE(result->direction == SortDirection::Ascending);
  }

  SECTION("Parse sort with explicit ascending (long form)") {
    auto result = ExpressionParser::parse_sort("priority ascending");
    REQUIRE(result.has_value());
    REQUIRE(result->field == SortField::Priority);
    REQUIRE(result->direction == SortDirection::Ascending);
  }

  SECTION("Parse sort with descending") {
    auto result = ExpressionParser::parse_sort("priority desc");
    REQUIRE(result.has_value());
    REQUIRE(result->field == SortField::Priority);
    REQUIRE(result->direction == SortDirection::Descending);
  }

  SECTION("Parse sort with descending (long form)") {
    auto result = ExpressionParser::parse_sort("priority descending");
    REQUIRE(result.has_value());
    REQUIRE(result->field == SortField::Priority);
    REQUIRE(result->direction == SortDirection::Descending);
  }

  SECTION("Parse sort with all supported fields") {
    REQUIRE(ExpressionParser::parse_sort("id").has_value());
    REQUIRE(ExpressionParser::parse_sort("title").has_value());
    REQUIRE(ExpressionParser::parse_sort("status").has_value());
    REQUIRE(ExpressionParser::parse_sort("priority").has_value());
    REQUIRE(ExpressionParser::parse_sort("created_date").has_value());
    REQUIRE(ExpressionParser::parse_sort("due_date").has_value());
  }

  SECTION("Parse sort with different fields and directions") {
    auto result1 = ExpressionParser::parse_sort("title asc");
    REQUIRE(result1.has_value());
    REQUIRE(result1->field == SortField::Title);
    REQUIRE(result1->direction == SortDirection::Ascending);

    auto result2 = ExpressionParser::parse_sort("created_date desc");
    REQUIRE(result2.has_value());
    REQUIRE(result2->field == SortField::CreatedDate);
    REQUIRE(result2->direction == SortDirection::Descending);

    auto result3 = ExpressionParser::parse_sort("id desc");
    REQUIRE(result3.has_value());
    REQUIRE(result3->field == SortField::Id);
    REQUIRE(result3->direction == SortDirection::Descending);
  }

  SECTION("Invalid sort expressions return nullopt") {
    REQUIRE_FALSE(ExpressionParser::parse_sort("").has_value());
    REQUIRE_FALSE(ExpressionParser::parse_sort("invalid_field").has_value());
  }

  SECTION("Unknown direction defaults to ascending with warning") {
    auto result = ExpressionParser::parse_sort("priority unknown_dir");
    REQUIRE(result.has_value());
    REQUIRE(result->field == SortField::Priority);
    REQUIRE(result->direction == SortDirection::Ascending);
  }
}

// ============================================================================
// Edge Cases and Robustness Tests
// ============================================================================

TEST_CASE("ExpressionParser edge cases", "[core][expr_parser]") {
  SECTION("Filter with >= should not be confused with >") {
    auto result = ExpressionParser::parse_filter("priority>=5");
    REQUIRE(result.has_value());
    REQUIRE(result->op == FilterOp::GreaterThanOrEqual);
    REQUIRE(result->value == "5");
  }

  SECTION("Filter with <= should not be confused with <") {
    auto result = ExpressionParser::parse_filter("priority<=1");
    REQUIRE(result.has_value());
    REQUIRE(result->op == FilterOp::LessThanOrEqual);
    REQUIRE(result->value == "1");
  }

  SECTION("Filter with value containing spaces") {
    auto result = ExpressionParser::parse_filter("title=Fix login bug");
    REQUIRE(result.has_value());
    REQUIRE(result->field == FilterField::Title);
    REQUIRE(result->value == "Fix login bug");
  }

  SECTION("Sort expression is case-sensitive for direction") {
    // Test that desc/asc are recognized (lowercase)
    auto result1 = ExpressionParser::parse_sort("priority desc");
    REQUIRE(result1.has_value());
    REQUIRE(result1->direction == SortDirection::Descending);

    auto result2 = ExpressionParser::parse_sort("priority asc");
    REQUIRE(result2.has_value());
    REQUIRE(result2->direction == SortDirection::Ascending);
  }
}
