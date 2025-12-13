#ifndef COMPILER_EXCEPTION_H
#define COMPILER_EXCEPTION_H

#include <stdexcept>
#include <string>

class CompilerException : public std::runtime_error {
public:
  CompilerException(const std::string &message) : std::runtime_error(message) {}

  CompilerException(const std::string &message, size_t position)
      : std::runtime_error(message + " at position " +
                           std::to_string(position)) {}
};

#endif // COMPILER_EXCEPTION_H
