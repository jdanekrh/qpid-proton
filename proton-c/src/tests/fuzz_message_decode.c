#include <stdint.h>

#include "proton/message.h"

// extern "C"
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    pn_message_t *msg = pn_message();
    int ret = pn_message_decode(msg, (const char *) Data, Size);
    if (ret == 0) {
        // do something, like encode msg and compare again with Data
    }
    if (msg != NULL) {
        pn_message_free(msg);
    }
    return 0;
}
