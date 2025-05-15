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
void send_spi(unsigned char *send_buff, unsigned char *rece_buff, int size);

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
void start_spi(unsigned char *device);
