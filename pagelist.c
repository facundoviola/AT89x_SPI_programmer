#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include "pagelist.h"

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
struct memorypage *get_page(struct memorypage *pagelist, int page_num)
{
    struct memorypage *aux = pagelist;

    while (1)
    {
        if (aux->p_address == page_num)
        {
            return aux;
        }

        if (aux->nextpage != NULL)
        {
            aux = aux->nextpage;
        }
        else
        {
            aux->nextpage = creat_page(page_num);
            return aux->nextpage;
        }
    }
}

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
struct memorypage *creat_page(int page_num)
{
    struct memorypage *aux;
    int i;

    aux = (struct memorypage *)malloc(sizeof(struct memorypage));
    aux->p_address = page_num;
    for (i = 0; i < 256; i++)
    {
        aux->data[i] = 0x00;
    }
    aux->nextpage = NULL;
    return aux;
}

/*
    Remove all nodes of the list and release the memory.

    Parameters
    ----------
    pagelist : struct memorypage *
        The nested list.
*/
void clear_list(struct memorypage *pagelist)
{
    struct memorypage *aux1, *aux2;
    aux1 = pagelist;

    while (1)
    {
        aux2 = aux1->nextpage;
        free(aux1);
        if (aux2 == NULL)
        {
            return;
        }
        aux1 = aux2;
    }
}

/*
    Dump to the screen the complete list in hexadecimal format.

    Parameters
    ----------
    pagelist : struct memorypage *
        The nested list.
*/
void print_list(struct memorypage *pagelist)
{
    struct memorypage *aux = pagelist;
    int i;

    while (1)
    {
        printf("Page %x\n", aux->p_address);

        for (i = 0; i < 256; i++)
        {
            printf("\tAdd %x - %x\n", i, aux->data[i]);
        }

        aux = aux->nextpage;
        if (aux == NULL)
        {
            return;
        }
    }
}
