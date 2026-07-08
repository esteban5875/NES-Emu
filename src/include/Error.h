#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>

extern noreturn void panic(const char* message, ...);