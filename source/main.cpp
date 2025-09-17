#include "cli/parser.hpp"
#include <iostream>

int main(int argc, char *argv[]) {
  // Parse command line arguments
  auto parsed = CommandParser::parse(argc, argv);

  // Handle parsing error
  if (!parsed.is_valid()) {
    if (!parsed.error_message.empty()) {
      std::cerr << "Error: " << parsed.error_message << "\n";
    }
    CommandParser::print_usage(argv[0]);
    return 1;
  }

  // Handle help command
  if (parsed.command == Command::Help) {
    CommandParser::print_help(argv[0]);
    return 0;
  }

  return 0;
}
