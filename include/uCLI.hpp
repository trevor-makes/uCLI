// Copyright (c) 2021 Trevor Makes

#pragma once

#include <stdint.h>

// Forward declaration of Arduino Stream type
struct Stream;

namespace uCLI {

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
};

using CommandFn = void (*)(Args);
using IdleFn = void (*)();

// Function pointer to be called when command string is entered
struct Command {
  const char* command;
  CommandFn callback;
};

// Read string from stream into buffer up to length bytes
void read_command(Stream& stream, char* buffer, uint8_t length, IdleFn idle_fn);

// Read string from stream into buffer
template <typename T, uint8_t BUF_LEN>
void read_command(T& stream, char (&buffer)[BUF_LEN], IdleFn idle_fn = nullptr) {
  read_command(stream, buffer, BUF_LEN, idle_fn);
}

// Attempt to match input to list of commands
void parse_command(Stream& stream, char* input, const Command* commands, uint8_t length);

// Attempt to match input to list of commands
template <typename T, uint8_t CMD_LEN>
void parse_command(T& stream, char* input, const Command (&commands)[CMD_LEN]) {
  parse_command(stream, input, commands, CMD_LEN);
}

// Display prompt and execute command from stream
template <uint8_t BUF_LEN = 80, typename T, uint8_t CMD_LEN>
void run_command(T& stream, const Command (&commands)[CMD_LEN], IdleFn idle_fn = nullptr) {
  char input[BUF_LEN];
  stream.write('>');
  read_command(stream, input, BUF_LEN, idle_fn);
  parse_command(stream, input, commands, CMD_LEN);
}

} // namespace uCLI
