#include "cli/parser.hpp"
#include <catch2/catch_test_macros.hpp>

// Helper function for testing simple commands (no arguments)
auto test_simple_command(const char *cmd_str, Command expected_cmd) {
  const char *args[] = {"taskproc", cmd_str};
  auto result = CommandParser::parse(2, const_cast<char **>(args));

  REQUIRE(result.is_valid());
  REQUIRE(result.command == expected_cmd);
  REQUIRE(result.args.empty());
}

// Verify command parsing
TEST_CASE("Command Parsing", "[cli][parser]") {
  SECTION("Basic load command") {
    const char *args[] = {"taskproc", "load", "tasks.csv"};
    auto result = CommandParser::parse(3, const_cast<char **>(args));

    REQUIRE(result.is_valid());
    REQUIRE(result.command == Command::Load);
    REQUIRE(result.args.size() == 1);
    REQUIRE(result.args[0] == "tasks.csv");
  }

  // Verify help command parsing
  SECTION("Help Command") { test_simple_command("help", Command::Help); }

  // Verify invalid command parsing
  SECTION("Invalid Command") {
    const char *args[] = {"taskproc", "invalid"};
    auto result = CommandParser::parse(2, const_cast<char **>(args));

    REQUIRE(!result.is_valid());
    REQUIRE(result.command == Command::Unknown);
    REQUIRE(!result.error_message.empty());
  }
}
//
// No-argument case: argc == 1 -> Help
TEST_CASE("CommandParser no-args returns Help", "[cli][parser]") {
  const char *args[] = {"taskproc"};
  auto result = CommandParser::parse(1, const_cast<char **>(args));
  REQUIRE(result.is_valid());
  REQUIRE(result.command == Command::Help);
  REQUIRE(result.args.empty());
}

// Verify command validation
TEST_CASE("Command validation", "[cli][parser]") {
  SECTION("Load command requires filename") {
    const char *args[] = {"taskproc", "load"};
    auto result = CommandParser::parse(2, const_cast<char **>(args));

    REQUIRE(!result.is_valid());
    REQUIRE(!result.error_message.empty());
  }

  SECTION("Validate commands without arguments") {
    test_simple_command("reload", Command::Reload);
    test_simple_command("clear", Command::Clear);
    test_simple_command("status", Command::Status);
    test_simple_command("list", Command::List);
  }
}
