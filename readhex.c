#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "pagelist.h"

#define DEBUGA 0
#define DEBUGB 0

void hexErrorFormat(char is_perror)
{
    if (is_perror)
        perror("hex file wrong format\n");
    else
        printf("hex file wrong format\n");
    exit(EXIT_FAILURE);
}

/*
    Read a line of and the HEX file

    Parameters
    ----------
    fd : int
        The file descriptor of the file to read
    s_address : int *
        Pointer to return the address of this line
    s_type : int *
        Pointer to return the type of this line
    s_lengthv : int *
        Pointer to return the amount of data bytes in this line
    d_out : unsigned char *
        The data buffer to store the bytes to read

    Return
    -------
    int: -1 -> the current line is an empty line, otherwise the record type.

    Note
    -------
    If hex file is not well formatted, checksum error exist or the record type is distinct from 0 or 1 the application will exit.
*/
int read_hex_line(int fd, int *s_address, int *s_type, int *s_length, unsigned char *d_out)
{
    int acum_data, i, aux;
    int record_length, address, record_type;
    unsigned char head[5], data[256 * 2], checksum, c_checksum;

    if ((i = read(fd, (void *)head, 1)) != 1)
    {
        if (i == 0)
            hexErrorFormat(0); // end of file
        else
            hexErrorFormat(1);
    }

    // newline
    if (head[0] == '\n' || head[0] == '\r')
    {
        return -1;
    }

    // colon
    if (head[0] != ':')
    {
        hexErrorFormat(0);
    }

    // record-length
    if (read(fd, (void *)head, 2) != 2)
    {
        hexErrorFormat(1);
    }
    head[2] = 0;
    sscanf(head, "%x", &record_length);
    if (DEBUGA)
        printf("record_length %d\n", record_length);

    // address
    if (read(fd, (void *)head, 4) != 4)
    {
        hexErrorFormat(1);
    }
    head[4] = 0;
    sscanf(head, "%x", &address);
    if (DEBUGA)
        printf("address %x\n", address);

    // record_type
    if (read(fd, (void *)head, 2) != 2)
    {
        hexErrorFormat(1);
    }
    head[2] = 0;
    sscanf(head, "%x", &record_type);
    if (DEBUGA)
        printf("record_type %x\n", record_type);

    if (record_type != 0 && record_type != 1)
    {
        printf("Record type (%d) not supported\n", record_type);
        exit(EXIT_FAILURE);
    }

    // data
    if (read(fd, (void *)data, 2 * record_length) != 2 * record_length)
    {
        hexErrorFormat(1);
    }

    head[2] = 0;

    acum_data = 0;

    for (i = 0; i < record_length; i++)
    {
        head[0] = data[2 * i];
        head[1] = data[2 * i + 1];
        sscanf(head, "%x", &aux);
        acum_data += aux;
        d_out[i] = aux;
    }

    // Get checksum
    if (read(fd, (void *)head, 2) != 2)
    {
        hexErrorFormat(1);
    }

    checksum = head[2] = 0;
    sscanf(head, "%x", &checksum);

    // Calc checksum
    acum_data = acum_data + record_length + record_type + ((address & 0xFF00) >> 8) + (address & 0xFF);
    c_checksum = ~(unsigned char)(0xff & acum_data);
    c_checksum += 1;

    if (DEBUGA)
    {
        printf("Checksum %x %x\n", c_checksum, checksum);
    }

    if (c_checksum != checksum)
    {
        hexErrorFormat(0);
    }

    *s_address = address;
    *s_type = record_type;
    *s_length = record_length;

    return record_type;
}

/*
    Read and process an HEX formatted file, return a nested list with each page (256 bytes) stored in each node of the list.

    Parameters
    ----------
    filename : char *
        The file to process

    Return
    -------
    struct memorypage *: The initial node of the nested list.

    Note
    -------
        If file is not possible to open the application will exit.
*/
struct memorypage *readhex(char *filename)
{

    int fd, page, i;
    int address, r_type, r_length;
    struct memorypage *pagelist = NULL;
    struct memorypage *currentpage;
    unsigned char data[256];

    if ((fd = open(filename, O_RDONLY)) == -1)
    {
        perror("fail open hex file");
        exit(EXIT_FAILURE);
    }

    // create page 0
    pagelist = creat_page(0);

    while (1)
    {
        if (read_hex_line(fd, &address, &r_type, &r_length, data) != -1)
        {
            page = (address & (~0xFF)) >> 8;
            address = address & 0xFF;

            currentpage = get_page(pagelist, page);

            for (i = 0; i < r_length; i++)
            {
                if ((i + address) >= 256)
                {
                    currentpage = get_page(pagelist, page + 1);
                    address = -i;
                }
                currentpage->data[i + address] = data[i];
            }

            if (r_type == 1)
                break;
            if (DEBUGB)
            {
                printf("Page %x Addr %x type %d len %d\n\n", page, address, r_type, r_length);
                /*
                for (page = 0; page < r_length; page++)
                {
                    printf("Data %d %x\n", page, data[page]);
                }
                */
            }
        }
    }

    close(fd);

    return pagelist;
}
