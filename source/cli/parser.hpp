#pragma once
#include <string>
#include <vector>

/// Available commands
enum class Command { Help, Load, Reload, Clear, Status, List, Unknown };

/// Struct representing parsed command-line arguments
struct ParsedArgs {
  Command command;
  std::vector<std::string> args;
  std::string error_message;

  bool is_valid() const { return command != Command::Unknown && error_message.empty(); }
};

/**
 * CommandParser class for parsing command-line arguments.
 *
 * @brief Parses command-line arguments and returns a ParsedArgs object.
 */
class CommandParser {
public:
  /**
   * Parses command-line arguments and returns a ParsedArgs object.
   *
   * @param argc Number of command-line arguments.
   * @param argv Array of command-line arguments.
   * @return ParsedArgs object containing parsed command and arguments.
   */
  static ParsedArgs parse(int argc, char *argv[]);

  /**
   * Prints help message for the program.
   *
   * @param program_name Name of the program.
   */
  static void print_help(const std::string &program_name);

  /**
   * Prints usage information for the program.
   *
   * @param program_name Name of the program.
   */
  static void print_usage(const std::string &program_name);

private:
  /**
   * Converts a string representation of a command to the corresponding Command enum value.
   *
   * @param cmd_str String representation of a command.
   * @return Command enum value corresponding to the input string.
   */
  static Command string_to_command(const std::string &cmd_str);
};
