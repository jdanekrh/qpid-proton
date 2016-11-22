#define _POSIX_C_SOURCE 200809L

#include <alloca.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>

#include "proton/url.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    char *str = alloca(Size + 1);
    memcpy(str, Data, Size);
    str[Size] = '\0';
    
    pn_url_t *url = pn_url_parse(str);
    if (url != NULL) {
        pn_url_free(url);
    }
    
    return 0;  // Non-zero return values are reserved for future use.
}
