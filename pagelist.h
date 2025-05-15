struct memorypage
{
    int p_address;
    unsigned char data[256];
    struct memorypage *nextpage;
};

/*
    Create a new page initialized with 0.

    Parameters
    ----------
    page_num : int
        The address of the requested page

    Return
    -------
    struct memorypage *: The node that represents the requested address.
*/
struct memorypage *creat_page(int);

/*
    Get a memory page from a page list, if the page already exist return its node,
    if not create a new node with the requested address and the data filled to 0.

    Parameters
    ----------
    pagelist : struct memorypage *
        The nested list.

    page_num : int
        The address of the requested page

    Return
    -------
    struct memorypage *: The node that represents the requested address.
*/
struct memorypage *get_page(struct memorypage *, int);

/*
    Remove all nodes of the list and release the memory.

    Parameters
    ----------
    pagelist : struct memorypage *
        The nested list.
*/
void clear_list(struct memorypage *);

/*
    Dump to the screen the complete list in hexadecimal format.

    Parameters
    ----------
    pagelist : struct memorypage *
        The nested list.
*/
void print_list(struct memorypage *);
