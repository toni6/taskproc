#pragma once
#include <string>

struct Task {
  int id;
  std::string title;

  Task(int id, std::string title) : id(id), title(std::move(title)) {}
};
