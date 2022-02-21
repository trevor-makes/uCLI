#include <Arduino.h>

#include "uCLI.hpp"

using uCLI::CLI;
using uCLI::StreamEx;
using uCLI::Tokens;

StreamEx serial_ex(Serial);
CLI<> serial_cli(serial_ex);

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
}

void do_add(StreamEx&, Tokens);
void do_echo(StreamEx&, Tokens);

void loop() {
  // command list can be global or static local
  static const uCLI::Command commands[] = {
    { "add", do_add }, // call do_add when "add" is entered
    { "echo", do_echo }, // call do_echo when "echo" is entered
  };
  serial_cli.prompt(commands);
}

void do_add(StreamEx& stream, Tokens args) {
  int a = atoi(args.next());
  int b = atoi(args.next());
  stream.print(a);
  stream.print(" + ");
  stream.print(b);
  stream.print(" = ");
  stream.println(a + b);
}

void do_echo(StreamEx& stream, Tokens args) {
  stream.println(args.next());
}
