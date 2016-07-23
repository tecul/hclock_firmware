#include "scheduler.h"
#include "gpio.h"
#include "systick.h"
#include "hclock.h"

#include <stdio.h>
#include <stdlib.h>

void construct_app()
{
    ;
}

void start_app()
{
    hclock_start();
}
