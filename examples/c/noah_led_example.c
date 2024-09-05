// SPDX-License-Identifier: GPL-2.0-or-later

/*
 * Broachlink Noah2/Noah3/Noah4/Noah5/Noah6 board platform
 * Sample code for managing the front LEDs through the /dev/noah
 *
 * Copyright (c) 2024 Rack Matrix Technology <wwww.rack-matrix.com>
 * Author: David RENÉ <developers_at_rack-matrix.com>
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>

#define LED1 1
#define LED2 2
#define LED3 3

int led_ON_OFF(int led, bool on_off)
{
    FILE *fd;
    char *led_path;
    int size = asprintf(&led_path, "/dev/noah/led%d", led);
    if (size == -1)
    {
        printf("led_ON_OFF can't allocate memory\n");
        return 1;
    }
    fd = fopen(led_path, "w");
    if (fd < 0)
    {
        perror("led_ON: ");
        return 1;
    }
    fprintf(fd, on_off ? "1" : "0");
    fclose(fd);
    free(led_path);
    return 0;
}

int led_ON(int led)
{
    return led_ON_OFF(led, true);
}

int led_OFF(int led)
{
    return led_ON_OFF(led, false);
}

int main()
{
    // set all leds to OFF
    led_OFF(LED1);
    led_OFF(LED2);
    led_OFF(LED3);

    while (1)
    {
        led_ON(LED1);
        usleep(50000);
        led_OFF(LED1);

        led_ON(LED2);
        usleep(50000);
        led_OFF(LED2);

        led_ON(LED3);
        usleep(50000);
        led_OFF(LED3);
    }
    return 0;
}
