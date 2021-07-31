// Copyright (c) 2021 Trevor Makes

#pragma once

#include <stdint.h>

struct Stream;

namespace uCLI {

class Args {
private:
  char* next_;

public:
  Args(char* args): next_{args} {}

  char* next();
  char* remainder();
};

// Function pointer to be called when command string is entered
struct Command {
  const char* command;
  void (*callback)(Args);
};

// Read string from stream into buffer up to length bytes
void read_command(Stream& stream, char* buffer, uint8_t length);

// Read string from stream into buffer
template <typename T, uint8_t L>
void read_command(T& stream, char (&buffer)[L]) {
    read_command(stream, buffer, L);
}

// Attempt to match input to list of commands
void parse_command(Stream& stream, char* input, const Command* commands, uint8_t length);

// Attempt to match input to list of commands
template <typename T, uint8_t L>
void parse_command(T& stream, char* input, const Command (&commands)[L]) {
    parse_command(stream, input, commands, L);
}

// Display prompt and execute command from stream
template <uint8_t N = 128, typename T, uint8_t L>
void run_command(T& stream, const Command (&commands)[L]) {
  char input[N];
  stream.write('>');
  read_command(stream, input, N);
  parse_command(stream, input, commands, L);
}

} // namespace uCLI
