#pragma once
#include <string>
#include <vector>

enum class Command { Help, Load, Reload, Clear, Status, List, Unknown };

struct ParsedArgs {
  Command command;
  std::vector<std::string> args;
  std::string error_message;

  bool is_valid() const {
    return command != Command::Unknown && error_message.empty();
  }
};

class CommandParser {
public:
  static ParsedArgs parse(int argc, char *argv[]);
  static void print_help(const std::string &program_name);
  static void print_usage(const std::string &program_name);

private:
  static Command string_to_command(const std::string &cmd_str);
};
