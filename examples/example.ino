#include "uCLI.hpp"

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
}

void do_foo(const char* args);
void do_bar(const char* args);

void loop() {
  // command list can be global or static local
  static const uCLI::Command commands[] = {
    { "foo", do_foo }, // run do_foo when "foo" is entered
    { "bar", do_bar }, // run do_bar when "bar" is entered
  };

  uCLI::run_command(Serial, commands);
}

void do_foo(const char* args) {
    Serial.write("Doing foo with: ");
    int value = atoi(args);
    Serial.println(value);
}

void do_bar(const char* args) {
    Serial.write("Doing bar with: ");
    Serial.println(args);
}