#include <stdlib.h>
#include <stdio.h>

#include "core/autodetect.h"

// extern "C" 
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    /* pni_protocol_type_t protocol = */ pni_sniff_header((const char *)Data, Size);
    return 0;  // Non-zero return values are reserved for future use.
}

// int main(int argc, char **argv) {
//     printf("Hello World!\n");
//     return 0;
// }
