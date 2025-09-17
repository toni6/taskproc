#include "cli/parser.hpp"
#include <iostream>
#include <unordered_map>

ParsedArgs CommandParser::parse(int argc, char *argv[]) {
  ParsedArgs result;

  // Need at least program name + command
  if (argc < 2) {
      result.command = Command::Help;
      return result;
  }

  std::string command = argv[1];
  result.command = string_to_command(command);

  if (result.command == Command::Unknown) {
      result.error_message = "Unknown command: " + command;
      return result;
  }

  // Collecting remaining arguments
  for (int i = 2; i < argc; ++i) {
      result.args.emplace_back(argv[i]);
  }

  // Validate argument count for each command
  switch (result.command) {
      case Command::Load:
          if (result.args.empty()) {
              result.error_message = "command 'load' requires a filename";
          }
          break;

      // Commands that don't require arguments
      case Command::Help:
      case Command::Reload:
      case Command::Clear:
      case Command::Status:
      case Command::List:
          break;

      case Command::Unknown:
          break; // Already handled above

  }

  return result;
}

Command CommandParser::string_to_command(const std::string &cmd_str) {
  static const std::unordered_map<std::string, Command> command_map = {
      {"help", Command::Help},
      {"load", Command::Load},
      {"reload", Command::Reload},
      {"clear", Command::Clear},
      {"status", Command::Status},
      {"list", Command::List},
  };

  auto it = command_map.find(cmd_str);
  if (it != command_map.end()) {
    return it->second;
  }
  return Command::Unknown;
}

void CommandParser::print_help(const std::string &program_name) {
  std::cout << "TaskProc CLI - Task Processing Tool\n\n";
  std::cout << "Usage: " << program_name << " [COMMAND] [OPTIONS]\n\n";
  std::cout << "Commands:\n";
  std::cout << "  help            Display this help message\n";
  std::cout << "  load <file>     Load tasks from a file\n\n";

  std::cout << "Examples:\n";
  std::cout << "  " << program_name << " load tasks.csv\n";
}

void CommandParser::print_usage(const std::string &program_name) {
  std::cout << "Usage: " << program_name << " [COMMAND] [OPTIONS]\n";
  std::cout << "Use '" << program_name << " help' for more information.\n";
}
