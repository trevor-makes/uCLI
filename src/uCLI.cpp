// https://github.com/trevor-makes/uCLI.git
// Copyright (c) 2021 Trevor Makes

#include "uCLI.hpp"
#include "uANSI.hpp"

#include <Arduino.h>
#include <string.h>

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

void parse_command(Stream& stream, char* input, const Command commands[], uint8_t length) {
  char* token = strtok(input, " ");

  // Look for match in command list
  for (uint8_t i = 0; i < length; ++i) {
    if (strcmp(token, commands[i].command) == 0) {
      char* message = strtok(nullptr, "");
      commands[i].callback(message);
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

} // namespace uCLI
