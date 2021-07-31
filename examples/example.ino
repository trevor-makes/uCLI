#include "uCLI.hpp"

void setup() {
  Serial.begin(9600);
  while (!Serial) {}
}

void do_add(uCLI::Args args);
void do_echo(uCLI::Args args);

void loop() {
  // command list can be global or static local
  static const uCLI::Command commands[] = {
    { "add", do_add }, // call do_add when "add" is entered
    { "echo", do_echo }, // call do_echo when "echo" is entered
  };

  uCLI::run_command(Serial, commands);
}

void do_add(uCLI::Args args) {
  int a = atoi(args.next());
  int b = atoi(args.next());
  Serial.print(a);
  Serial.print(" + ");
  Serial.print(b);
  Serial.print(" = ");
  Serial.println(a + b);
}

void do_echo(uCLI::Args args) {
  Serial.println(args.remainder());
}
