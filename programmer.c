#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include "readhex.h"
#include "pagelist.h"
#include "spi.h"

#define DEABUGB1 1 // Connect Info
#define DEABUGB2 1 // Erase Info
#define DEABUGC 0  // Show Red Page

#define ENABLE1 0b10101100
#define ENABLE2 0b01010011
#define ENABLE3 0b00000000
#define READPAGE 0b00110000
#define WRITEPAGE 0b01010000
#define CHIPERASE 0b10000000

unsigned char ENABLE_SP[] = {ENABLE1, ENABLE2, ENABLE3, ENABLE3};
unsigned char ERASE_SP[] = {ENABLE1, CHIPERASE, ENABLE3, ENABLE3};

void clear_mem(unsigned char *rece_buf, int size)
{
    int i;
    for (i = 0; i < size; i++)
        *(rece_buf + i) = 0x00;
}

void copy_mem(unsigned char *A, unsigned char *B, int size)
{
    int i;
    for (i = 0; i < size; i++)
        *(A + i) = *(B + i);
}

int comp_mem(unsigned char *A, unsigned char *B, int size)
{
    int i;
    for (i = 0; i < size; i++)
    {
        if (*(A + i) != *(B + i))
            return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{

    unsigned char *rece_buf, *send_buf;
    int s_state, i, wr_page;

    int PROGRAM = 0;

    struct memorypage *pagelist;

    if (argc != 3 && argc != 4)
    {
        printf("Usage: %s <device> <hex file> [program]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    if ((argc == 4) && (strcmp(argv[3], "program") == 0))
        PROGRAM = 1;

    pagelist = readhex(argv[2]);

    rece_buf = (unsigned char *)malloc(4);

    start_spi(argv[1]);

    do
    {
        printf("Verify RS232 cable is connected and switch is in the program position (UP) then press enter.");
        getchar();
        clear_mem(rece_buf, 4);
        send_spi(ENABLE_SP, rece_buf, 4);
        if (DEABUGB1)
            printf("Connect Status Code %x\n", rece_buf[3]);

        if (rece_buf[3] == 0x69)
            break;

    } while (1);

    /* Chip Erase */
    if (PROGRAM)
    {
        clear_mem(rece_buf, 4);
        send_spi(ERASE_SP, rece_buf, 4);
        if (DEABUGB2)
            printf("Erase Status Code %x:%x:%x:%x\n", rece_buf[0], rece_buf[1], rece_buf[2], rece_buf[3]);
    }

    free(rece_buf);

    send_buf = (unsigned char *)malloc(258);
    rece_buf = (unsigned char *)malloc(258);

    while (1)
    {
        wr_page = pagelist->p_address;

        if (PROGRAM)
        {
            printf("Writing page %x\n", wr_page);
            clear_mem(send_buf, 258);
            send_buf[0] = WRITEPAGE;
            send_buf[1] = wr_page;
            copy_mem(send_buf + 2, pagelist->data, 255);
            send_spi(send_buf, rece_buf, 258);
        }

        /* Read Page */
        printf("Verification of page %x\n", wr_page);

        clear_mem(send_buf, 258);
        clear_mem(rece_buf, 258);

        send_buf[0] = READPAGE;
        send_buf[1] = wr_page;

        send_spi(send_buf, rece_buf, 258);

        if (DEABUGC)
        {
            printf("--Page--%x\n", wr_page);

            for (i = 2; i < 258; i++)
                printf("Byte %x %x\n", i - 2, *(rece_buf + i));
        }

        if (comp_mem(rece_buf + 2, pagelist->data, 256) != 0)
        {
            printf("Verification page %x error\n", wr_page);
            break;
        }

        if (pagelist->nextpage != NULL)
            pagelist = pagelist->nextpage;
        else
        {
            printf("------------Verification Success------------\n");
            break;
        }
    }

    free(send_buf);
    free(rece_buf);

    clear_list(pagelist);
    return 0;
}
