/* The MIT License
   
   Copyright (c) 2010 David Soria Parra <dsp experimentalworks net>
   
   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:
   
   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.
   
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/kd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>

#define PROGRAM_NAME "mled"

/* LED_SCR = 0x1, LED_NUM = 0x2, LED_CAP = 0x4 */

#define LEDS LED_CAP
#define ERROR -1
#define SUCCESS 0
#define S 250000 /* Short dit, 0.5s */
#define L 3*S    /* Long  dah, 1.5s = s*3 */
#define WS 7*S   /* 7*1.5s break between words */
#define N -1
#define verbose(...) if(opt_verbose) { fprintf(stdout, __VA_ARGS__); fflush(stdout);};

static int fd = 0, opt_verbose = 0, opt_time = 0;
static long int saved_led_state = 0x0;

#define MORSE_DIM 5
int morsetable[][MORSE_DIM] = {
    {S, L, N, N, N},  /* a */
    {L, S, S, N, N},  /* b */
    {L, S, L, S, N},  /* c */
    {L, S, S, N, N},  /* d */
    {S, N, N, N, N},  /* e */
    {S, S, L, S, N},  /* f */
    {L, L, S, N, N},  /* g */
    {S, S, S, S, N},  /* h */
    {S, S, N, N, N},  /* i */
    {S, L, L, L, N},  /* j */
    {L, S, L, N, N},  /* k */
    {S, L, S, S, N},  /* l */
    {L, L, N, N, N},  /* m */
    {L, S, N, N, N},  /* n */
    {L, L, L, N, N},  /* o */
    {S, L, L, S, N},  /* p */
    {L, L, S, L, N},  /* q */
    {S, L, S, N, N},  /* r */
    {S, S, S, N, N},  /* s */
    {L, N, N, N, N},  /* t */
    {S, S, L, N, N},  /* u */
    {S, S, S, L, N},  /* v */
    {S, L, L, N, N},  /* w */
    {L, S, S, L, N},  /* x */
    {L, S, L, L, N},  /* y */
    {L, L, S, S, N},  /* z */
    {L, L, L, L, L},  /* 0 */
    {S, L, L, L, L},  /* 1 */
    {S, S, L, L, L},  /* 2 */
    {S, S, S, L, L},  /* 3 */
    {S, S, S, S, L},  /* 4 */
    {S, S, S, S, S},  /* 5 */
    {L, S, S, S, S},  /* 6 */
    {L, L, S, S, S},  /* 7 */
    {L, L, L, S, S},  /* 8 */
    {L, L, L, L, S}}; /* 9 */

static void led(long int led_id) {
    if ((ioctl(fd, KDSETLED, led_id)) == ERROR) {
        perror("ioctl");
        close(fd);
        exit(ERROR);
    }
}

static void stop(long int len) {
    led(0x0);
    usleep(S);
}

static void blink(long int led_id, long int len) {
    led(led_id);
    usleep(len);

    /* make a pause of S after every symbol */
    stop(S);
}

/* morse a single ascii character as a seq of symbols */
static void morse(int led_id, int tomorse[]) {
    int i;
    for (i = 0; i < MORSE_DIM && tomorse[i] != -1; i++) {
        if (tomorse[i] == L)
            verbose("-");
        if (tomorse[i] == S) 
            verbose(".");
        blink(led_id, tomorse[i]);
    }
    verbose(" ");

    /* make a long pause after every character */
    stop(L);
}

static void backupleds(long int *backup) {
    if ((ioctl(fd, KDGETLED, backup)) == ERROR) {
        perror("ioctl");
        close(fd);
        exit(ERROR);
    }
}

static void help() {
    printf("Usage: %s [OPTIONS] [FILE]\n", PROGRAM_NAME);
    printf("Sends a message from stdin as morsecode to your CAPS LOCK led\n");
    printf("\n\
  -v                       verbose output\n\
  -t                       show time instead of reading from stdin\n\
  -h                       show help\n\n");
}

void sighandler(int signum) {
    if ((ioctl(fd, KDSETLED, saved_led_state)) == ERROR) {
        perror("ioctl");
        close(fd);
        exit(ERROR);
    }

    close(fd);
    exit(0);
}

void get_time(char *now_h, char *now_m) {
    struct tm *ptr;
    time_t lt;

    lt = time(NULL);
    ptr = localtime(&lt);

    strftime(now_h, 100, "%H", ptr);
    strftime(now_m, 100, "%M", ptr);
}

void blink_binary(int num) {
    if (num & 1) {
        printf("1");
        blink(LEDS, L);
    } else {
        printf("0");
        blink(LEDS, S);
    }
    if (num & 2) {
        printf("1");
        blink(LEDS, L);
    } else {
        printf("0");
        blink(LEDS, S);
    }
    if (num & 4) {
        printf("1");
        blink(LEDS, L);
    } else {
        printf("0");
        blink(LEDS, S);
    }
    if (num & 8) {
        printf("1");
        blink(LEDS, L);
    } else {
        printf("0");
        blink(LEDS, S);
    }
    if (num & 16) {
        printf("1");
        blink(LEDS, L);
    } else {
        printf("0");
        blink(LEDS, S);
    }
    if (num & 32) {
        printf("1");
        blink(LEDS, L);
    } else {
        printf("0");
        blink(LEDS, S);
    }

    printf("\n");
}

int main(int argc, char * argv[]) {
    int in, n_h, n_m;
    char opt;
    size_t s, i;
    char *filename = NULL, buf[1024];
    char now_h[3];
    char now_m[3];

    if (geteuid() != 0) {
        fprintf(stderr, "program must be run as superuser\n");
        exit(ERROR);
    }

    /* register sighandlers to restore LED state */
    signal(SIGINT,  sighandler);
    signal(SIGTERM, sighandler);
    signal(SIGQUIT, sighandler);
    signal(SIGTSTP, sighandler);

    if ((fd = open("/dev/console", O_NOCTTY)) == ERROR) {
        perror("open");
        exit(ERROR);
    }

    while((opt = getopt(argc, argv, "hvt")) > 0) {
        switch (opt) {
            case 'h':
                help();
                exit(SUCCESS);
                break;
            case 'v':
                opt_verbose = 1;
                break;
            case 't':
                opt_time = 1;
                break;
        }
    }

    argc-= optind;
    argv+= optind;
    /* argument parsing */
    if (argc > 1) {
        filename = argv[1];
        if ((in = open(filename, O_RDONLY)) == ERROR) {
            perror("open");
            exit(ERROR);
        }
    } else {
        in = STDIN_FILENO;
    }

    backupleds(&saved_led_state);

    if (opt_time > 0) {
        get_time(now_h, now_m);
        printf("TIME: %s:%s \n", now_h, now_m);
        n_h = atoi(now_h); 
        n_m = atoi(now_m);
        printf("%d %d\n", n_h, n_m);
        blink_binary(n_h);
        blink(LEDS, 6*S);
        blink_binary(n_m);
    } else {

    /* start morsing */
    while ((s = read(in, &buf, 1024)) > 0) {
        for (i = 0; i < s; i++) {
            if (buf[i] >= 'A' && buf[i] <= 'Z') {
                morse(LEDS, morsetable[buf[i] - 'A']);
            } else if (buf[i] >= 'a' && buf[i] <= 'z') {
                morse(LEDS, morsetable[buf[i] - 'a']);
            } else if (buf[i] >= '0' && buf[i] <= '9') {
                morse(LEDS, morsetable[buf[i] - '0' + 26]);
            } else if (buf[i] == ' ') {
                stop(WS);
            } else {
                fprintf(stderr, "non-ascii character found\n");
            }
        }
    }

    }

    led(saved_led_state);

    close(in);
    close(fd);

    return 0;
}

