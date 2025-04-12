#pragma once

#include "../../../inc/esphdrs.h"

#define SHELL_ERROR -1

int shellmain_relay(int argc, char **argv);
int shellmain_led(int argc, char **argv);
int shellmain_echo(int argc, char **argv);
int shellmain_wifi(int argc, char **argv);
int shellmain_shell(int argc, char **argv);
int shellmain_termo(int argc, char **argv);
int shellmain_ls(int argc, char **argv);
int shellmain_pwd(int argc, char **argv);
int shellmain_cd(int argc, char **argv);
int shellmain_cat(int argc, char **argv);
int shellmain_server(int argc, char **argv);