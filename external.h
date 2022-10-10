#ifndef EXTERNAL_H
#define EXTERNAL_H

#include <stdbool.h>

typedef enum AsmLocationType
{
    LOCATION_NONE,
    LOCATION_REG,
    LOCATION_STR,
} AsmLocationType;

typedef struct AsmLocation 
{
    AsmLocationType type;
    char *str;
    char reg;
} AsmLocation;

typedef struct AsmPlace
{
    bool one_location;
    AsmLocation locations[2];
    struct AsmPlace *next;
} AsmPlace;

AsmLocation location_register(char reg);
AsmLocation location_string(char *str);
AsmLocation location_none();

AsmPlace *place_double(AsmLocation location_low, AsmLocation location_high);
AsmPlace *place_single(char *str);

AsmPlace *place_append_list(AsmPlace *list, AsmPlace *to_append);

void free_place(AsmPlace *asm_place);

#endif
