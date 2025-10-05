#include "task.hpp"
#include <iostream>

std::ostream &operator<<(std::ostream &os, const Task &task) {
  os << "ID: " << task.id << " | Title: " << task.title << " | Status: " << task.status
     << " | Priority: " << task.priority;
  return os;
}
