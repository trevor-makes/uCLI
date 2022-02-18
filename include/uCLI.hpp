// Copyright (c) 2021 Trevor Makes

#pragma once

#include "uANSI.hpp"

#include <stdint.h>

namespace uCLI {

using uANSI::StreamEx;

class Args {
private:
  char* next_;

public:
  Args(char* args): next_{args} {}

  // Casting const literal "" to char* is a necessary evil for now so that we
  // can avoid dealing with nullptr; the empty string will NOT be mutated
  Args(): next_{const_cast<char*>("")} {}

  const char* next();
  bool has_next() { return *next_ != '\0'; }
  bool is_string() { return *next_ == '\"' || *next_ == '\''; }

  template <uint8_t N>
  uint8_t get(const char* (&argv)[N], bool are_strings[] = nullptr) {
    for (uint8_t i = 0; i < N; ++i) {
      if (!has_next()) {
        return i;
      }
      if (are_strings != nullptr) {
        are_strings[i] = is_string();
      }
      argv[i] = next();
    }
    return N;
  }
};

using CommandFn = void (*)(Args);
using IdleFn = void (*)();

// Function pointer to be called when command string is entered
struct Command {
  const char* command;
  CommandFn callback;
};

// Read string from stream into buffer up to length bytes
void read_command(StreamEx& stream, char* buffer, uint8_t length, IdleFn idle_fn);

// Read string from stream into buffer
template <uint8_t BUF_LEN>
void read_command(StreamEx& stream, char (&buffer)[BUF_LEN], IdleFn idle_fn = nullptr) {
  read_command(stream, buffer, BUF_LEN, idle_fn);
}

// Attempt to match input to list of commands
void parse_command(StreamEx& stream, char* input, const Command* commands, uint8_t length);

// Attempt to match input to list of commands
template <uint8_t CMD_LEN>
void parse_command(StreamEx& stream, char* input, const Command (&commands)[CMD_LEN]) {
  parse_command(stream, input, commands, CMD_LEN);
}

// Display prompt and execute command from stream
template <uint8_t BUF_LEN = 80, uint8_t CMD_LEN>
void run_command(StreamEx& stream, const Command (&commands)[CMD_LEN], IdleFn idle_fn = nullptr) {
  static char input[BUF_LEN];
  stream.write('>');
  read_command(stream, input, BUF_LEN, idle_fn);
  parse_command(stream, input, commands, CMD_LEN);
}

} // namespace uCLI
