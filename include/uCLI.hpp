// https://github.com/trevor-makes/uCLI.git
// Copyright (c) 2021 Trevor Makes

#pragma once

#include <stdint.h>

struct Stream;

namespace uCLI {

// Function pointer to be called when command string is entered
struct Command {
  const char* command;
  void (*callback)(const char*);
};

// Read string from stream into buffer up to length bytes
void read_command(Stream& stream, char* buffer, uint8_t length);

// Read string from stream into buffer
template <typename T, int L>
void read_command(T& stream, char (&buffer)[L]) {
    read_command(stream, buffer, L);
}

// Attempt to match input to list of commands
void parse_command(Stream& stream, char* input, const Command* commands, uint8_t length);

// Attempt to match input to list of commands
template <typename T, int L>
void parse_command(T& stream, char* input, const Command (&commands)[L]) {
    parse_command(input, commands, L);
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
