#include "external.h"
#include <stdio.h>
#include <stdlib.h>

AsmLocation location_register(char reg)
{
    AsmLocation loc = { LOCATION_REG, NULL, reg };
    return loc;
}

AsmLocation location_string(char *str)
{
    AsmLocation loc = { LOCATION_STR, str, 0 };
    return loc;
}

AsmLocation location_none()
{
    AsmLocation loc = { LOCATION_NONE, NULL, 0 };
    return loc;
}

AsmPlace *place_double(AsmLocation location_low, AsmLocation location_high)
{

    AsmPlace *place = malloc(sizeof(AsmPlace));
    place->one_location = false;
    place->locations[0] = location_low;
    place->locations[1] = location_high;
    place->next = NULL;
    return place;
}

AsmPlace *place_single(char *str)
{
    AsmPlace *place = place_double(location_string(str), location_none());
    place->one_location = true;
    return place;
}

AsmPlace *place_append_list(AsmPlace *list, AsmPlace *to_append)
{
    fprintf(stderr, "here\n");
    AsmPlace *curr = list;
    while (curr->next)
    {
        curr = curr->next;
        fprintf(stderr, "next: %p\n", curr->next);
    }
    curr->next = to_append;
    return list;
}

void free_place(AsmPlace *asm_place)
{
    while (asm_place)
    {
        AsmPlace *next = asm_place->next;
        for (int i = 0; i < 2; ++i)
            if (asm_place->locations[i].str)
                free(asm_place->locations[i].str);
        free(asm_place);
        asm_place = next;
    }
}
