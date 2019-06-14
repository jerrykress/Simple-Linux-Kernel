#ifndef __PX_H
#define __PX_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "PL011.h"

#define MUTX_LOCKED (0)
#define MUTX_UNLOCKED (1)
#define MUTX_WRITE_OK (2)
#define MUTX_RELEASE_OK (3)
#define MUTX_RELEASE_FAIL (4)
#define MUTX_READ_FAIL (-1)
#define MUTX_WRITE_FAIL (5)
#define MUTX_ERROR (-9)

typedef int pid_t;

#endif