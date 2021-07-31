// Copyright (c) 2021 Trevor Makes

#include "uCLI.hpp"
#include "uANSI.hpp"

#include <Arduino.h>

namespace uCLI {

void read_command(Stream& stream, char* buffer, uint8_t length) {
  uint8_t i = 0;

  do {
    // Get next key press
    int input = uANSI::read_key(stream);
    if (input == -1) {
      continue;
    }

    // Handle special keys (arrows, etc)
    if (input == '\e' || input > 0xFF) {
      continue;
    }

    // Handle backspace and delete
    if (input == '\x08' || input == '\x7F') {
      if (i > 0) {
        // Delete last character
        stream.write("\e[D\e[X");
        --i;
      }
      continue;
    }

    // Handle enter/return
    if (input == '\n' || input == '\r') {
      if (i > 0) {
        // Exit loop and execute command
        break;
      } else {
        continue;
      }
    }

    // Echo and record input
    stream.write(input);
    buffer[i++] = input;
  } while (i < length);

  stream.write("\n");
  buffer[i] = '\0';
}

char* trim_space(char* input) {
  while (*input == ' ') {
    ++input;
  }
  return input;
}

char* split_at_space(char* input) {
  // Scan until end of string
  while (*input != '\0') {
    // Replace space with null and return
    if (*input == ' ') {
      *input++ = '\0';
      return trim_space(input);
    }
    ++input;
  }
  return input;
}

void parse_command(Stream& stream, char* input, const Command commands[], uint8_t length) {
  input = trim_space(input);
  Args args = split_at_space(input);

  // Look for match in command list
  for (uint8_t i = 0; i < length; ++i) {
    if (strcmp(input, commands[i].command) == 0) {
      commands[i].callback(args);
      return;
    }
  }

  // Invalid command; print command list
  stream.write("Commands: ");
  for (uint8_t i = 0; i < length; ++i) {
    if (i > 0) {
      stream.write(", ");
    }
    stream.write(commands[i].command);
  }
  stream.write("\n");
}

char* Args::next() {
  char* next = next_;
  next_ = split_at_space(next_);
  return next;
}

char* Args::remainder() {
  return next_;
}

} // namespace uCLI
