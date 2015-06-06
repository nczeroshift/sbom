
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string>

#include "bxon.h"

int main(int argv, char * argc[])
{
    bxon_object * map = bxon_map_new(10);

    bxon_map_put(map,"int",bxon_new_int(10));
    bxon_map_put(map,"long",bxon_new_long(1e8));
    bxon_map_put(map,"string",bxon_new_string("bxon test string"));
    bxon_map_put(map,"float",bxon_new_float(10.000001f));
    bxon_map_put(map,"double",bxon_new_double(12.000001f));
    bxon_map_put(map,"boolean",bxon_new_bool(0));
    bxon_map_put(map,"bool",bxon_new_bool(0));
    bxon_map_put(map,"byte",bxon_new_byte(0xFA));

    bxon_object * array = bxon_array_new(BXON_FLOAT,3);
    bxon_array_push(array,bxon_new_float(0.01f));
    bxon_array_push(array,bxon_new_float(0.02f));
    bxon_array_push(array,bxon_new_float(0.03f));

    bxon_map_put(map,"array",array);

    bxon_release(&map);

	return 0;
}
