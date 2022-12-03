#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <inttypes.h>

#include "utils/so_scheduler.h"
#include "utils/utils.h"

so_scheduler_t *schedpreemt;
