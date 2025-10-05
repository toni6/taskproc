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

  case Command::Sort:
    break;

  case Command::Filter:
    if (result.args.empty()) {
      result.error_message = "command 'filter' requires a filter expression";
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

Command CommandParser::string_to_command(std::string_view cmd_str) {
  static const std::unordered_map<std::string_view, Command> command_map = {{"help", Command::Help},
                                                                            {"load", Command::Load},
                                                                            {"reload", Command::Reload},
                                                                            {"clear", Command::Clear},
                                                                            {"status", Command::Status},
                                                                            {"list", Command::List},
                                                                            {"filter", Command::Filter},
                                                                            {"sort", Command::Sort}};

  auto it = command_map.find(cmd_str);
  if (it != command_map.end()) {
    return it->second;
  }
  return Command::Unknown;
}

void CommandParser::print_help(std::string_view program_name) {
  std::cout << "TaskProc CLI - Task Processing Tool\n\n";
  std::cout << "Usage: " << program_name << " [COMMAND] [OPTIONS]\n\n";
  std::cout << "Commands:\n";
  std::cout << "  help            Display this help message\n";
  std::cout << "  load <file>     Load tasks from a file\n";
  std::cout << "  reload          Reload tasks from the last loaded file\n";
  std::cout << "  list            List current task view\n";
  std::cout << "  clear           Reset task view\n";
  std::cout << "  sort            Sort tasks by priority\n";
  std::cout << "  filter          Filter tasks by status\n";

  std::cout << "\nExamples:\n";
  std::cout << "  " << program_name << " load tasks.csv\n";
  std::cout << "  " << program_name << " filter status=todo\n";
  std::cout << "  " << program_name << " sort priority desc\n";
}

void CommandParser::print_usage(std::string_view program_name) {
  std::cout << "Usage: " << program_name << " [COMMAND] [OPTIONS]\n";
  std::cout << "Use '" << program_name << " help' for more information.\n";
}
