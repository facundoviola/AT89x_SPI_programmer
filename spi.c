#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>

#include <sys/sem.h>
#include <sys/ioctl.h>

#define PROGRESS_BAR

#define DEABUGA 0

#define SIGNALNUM SIGRTMIN
#define BPS 6000000 // Clock period in nanoseconds
#define NSEC (int)BPS / 5

// Pin 5 GND RS-232
#define CLK TIOCM_RTS  // Pin 7 RS-232
#define MOSI TIOCM_DTR // Pin 4 RS-232
#define MISO TIOCM_CTS // Pin 8 RS-232

#define LOW 0
#define RISE 1
#define UP 2
#define FALL 3
#define P_LOW 4

#define SEMKEY 890698677

struct termios termios_p;
struct sigevent sev;
struct sigaction sa;

struct itimerspec itrun, itstop;
int fd, semid;
timer_t timerid;

/* Handler Variables */
int clk = LOW;     // Current output clock phase
int h_byte, h_bit; // Write/Read pos
int h_finished, bytes_size;
unsigned char *misobuf, *mosibuf;

#ifdef PROGRESS_BAR
unsigned char progress_b[27];
int progress_n;
#endif

/*
   Timer signal handler.
*/
void handler(int sig)
{
    int aux_s, aux_r, s_state;
    struct sembuf sops;

    if (h_finished)
        return;

    ioctl(fd, TIOCMGET, &s_state);

    if (clk == RISE)
    {

#ifdef PROGRESS_BAR
        if ((h_byte != 0) && (h_byte % 10 == 0) && (h_bit == 7))
        {
            progress_b[progress_n] = '*';
            progress_b[progress_n + 1] = 0;
            progress_n++;
            printf("%s\n", progress_b);
        }
#endif

        aux_r = (s_state & MISO) == 0;
        misobuf[h_byte] |= aux_r << h_bit;

        if (DEABUGA)
            printf("\tByte %d Bit %d Get %d\n", h_byte, h_bit, aux_r);

        s_state &= ~CLK;
        clk = UP;
    }
    else if (clk == UP)
    {
        clk = FALL;
    }
    else if (clk == FALL)
    {
        s_state |= CLK;
        clk = P_LOW;
    }
    else if (clk == P_LOW)
    {
        h_bit--;
        if (h_bit < 0)
        {
            h_byte++;
            h_bit = 7;

            if (h_byte >= bytes_size)
            {
                h_finished = 1;
                sops.sem_num = 0;
                sops.sem_op = -1;

                if (semop(semid, &sops, 1) == -1)
                {
                    perror("semop C error");
                    exit(EXIT_FAILURE);
                }
            }
        }

        clk = LOW;
    }
    else if (clk == LOW)
    {
        aux_s = mosibuf[h_byte];
        aux_s = (aux_s & (1 << h_bit)) == 0;
        if (DEABUGA)
            printf("Byte %d Bit %d Send %d\n", h_byte, h_bit, !aux_s);

        if (aux_s)
            s_state |= MOSI;
        else
            s_state &= ~MOSI;

        clk = RISE;
    }

    ioctl(fd, TIOCMSET, &s_state);
}

/*
    Send and receive (size) bytes to the SPI interface.

    Parameters
    ----------
    send_buff/rece_buff : unsigned char *
        The send and receive buffer.

    size : int
        The size in bytes of the buffers.

    Note
    -------
    If some problem arise while executing this function an error code will be printed and the application will exit.
*/
void send_spi(unsigned char *send_buff, unsigned char *rece_buff, int size)
{
    struct sembuf sops;
    int sem_status;

    mosibuf = send_buff;
    misobuf = rece_buff;
    clk = LOW;
    h_finished = 0;
    h_byte = 0;
    h_bit = 7;
    bytes_size = size;

#ifdef PROGRESS_BAR
    progress_b[0] = 0;
    progress_n = 0;
#endif

    sops.sem_num = 0;
    sops.sem_op = 1;

    if (semop(semid, &sops, 1) == -1)
    {
        perror("semop A error");
        exit(EXIT_FAILURE);
    }

    if (timer_settime(timerid, 0, &itrun, NULL) == -1)
    {
        perror("timer_settime error");
        exit(EXIT_FAILURE);
    }

    sops.sem_op = 0;

    do
    {
        sem_status = semop(semid, &sops, 1);
    } while (sem_status != 0);

    if (timer_settime(timerid, 0, &itstop, NULL) == -1)
    {
        perror("timer_settime error");
        exit(EXIT_FAILURE);
    }

    return;
}

/*
    Start the SPI interface.

    Parameters
    ----------
    device : unsigned char *
       The name of the serial device to use as an SPI interface.

    Note
    -------
    If some problem arise while executing this function an error code will be printed and the application will exit.
*/
void start_spi(unsigned char *device)
{
    int s_state;

    /* Open device */
    if ((fd = open(device, O_RDWR | O_NDELAY)) < 0)
    {
        printf("Error while open %s", device);
        perror(NULL);
        exit(EXIT_FAILURE);
    }

    /* Set Serial port in Raw Mode */
    cfmakeraw(&termios_p);
    if (tcsetattr(fd, TCSANOW, &termios_p) < 0)
    {
        printf("Error while tcsetattr");
        perror(NULL);
        exit(EXIT_FAILURE);
    }

    /* Clear clock signal */
    ioctl(fd, TIOCMGET, &s_state);
    s_state |= CLK;
    ioctl(fd, TIOCMSET, &s_state);

    /* Set signal Handler */
    memset(&sa, 0, sizeof(struct sigaction));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGNALNUM, &sa, NULL) == -1)
    {
        perror("sigaction error");
        exit(EXIT_FAILURE);
    }

    /* Create the timer */
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGNALNUM;
    if (timer_create(CLOCK_MONOTONIC, &sev, &timerid) == -1)
    {
        perror("timer_create error");
        exit(EXIT_FAILURE);
    }

    /* Set the timers values */
    itrun.it_value.tv_nsec = NSEC;
    itrun.it_value.tv_sec = 0;
    itrun.it_interval.tv_sec = itrun.it_value.tv_sec;
    itrun.it_interval.tv_nsec = itrun.it_value.tv_nsec;

    itstop.it_value.tv_nsec = 0;
    itstop.it_value.tv_sec = 0;
    itstop.it_interval.tv_sec = itstop.it_value.tv_sec;
    itstop.it_interval.tv_nsec = itstop.it_value.tv_nsec;

    if ((semid = semget(SEMKEY, 1, IPC_CREAT | 0600)) == -1)
    {
        perror("semget error");
        exit(EXIT_FAILURE);
    }

    if (semctl(semid, 0, SETVAL, 0) == -1)
    {
        perror("semctl error");
        exit(EXIT_FAILURE);
    }
}
