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
struct memorypage *readhex(char *);
