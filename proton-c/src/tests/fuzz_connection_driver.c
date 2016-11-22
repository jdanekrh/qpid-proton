#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#include "proton/connection_driver.h"
#include "proton/engine.h"
#include "proton/message.h"


#define MAX_SIZE 1024


typedef char str[MAX_SIZE];

typedef struct app_data_t {
  str container_id;
  pn_rwbytes_t message_buffer;
  int message_count;
  int received;
} app_data_t;

static void fdc_write(pn_connection_driver_t *driver);
size_t fcd_read(pn_connection_driver_t *driver, uint8_t **data, size_t *size);
static void decode_message(pn_delivery_t *dlv);
static void handle(app_data_t* app, pn_event_t* event);
static void check_condition(pn_event_t *e, pn_condition_t *cond);


const bool VERBOSE = true;
const bool ERRORS = true;


// extern "C" 
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
  app_data_t app = {{0}};
  snprintf(app.container_id, sizeof(app.container_id), "%s:%d", "fuzz_connection_driver", getpid());
  
   pn_connection_driver_t driver; 
    if (pn_connection_driver_init(&driver, NULL, NULL) != 0) {
        printf("pn_connection_driver_init\n");
        exit(1);
    }
    // The transport is bound automatically after the PN_CONNECTION_INIT has been is
    // handled by the application.
//     pn_connection_driver_bind(&driver); // WAT?

    uint8_t * data = (uint8_t *) Data;
    size_t size = Size;
    
    fdc_write(&driver);
    
    pn_event_t *event;
    while ((event = pn_connection_driver_next_event(&driver)) != NULL) {
            handle(&app, event);
    }
    
    fdc_write(&driver);
        
    do {
        fcd_read(&driver, &data, &size);
        printf("size is %d, data is %p\n", (int) size, (void *) data);
        fdc_write(&driver);
        pn_event_t *event;
        while ((event = pn_connection_driver_next_event(&driver)) != NULL) {
            handle(&app, event);
        }
    } while (size > 0);
    
    pn_connection_driver_close(&driver);
    pn_connection_driver_destroy(&driver);  // documentation says pn_connection_driver_free
  return 0;
}

static void handle(app_data_t* app, pn_event_t* event) {
  switch (pn_event_type(event)) {

   case PN_CONNECTION_INIT: {
     pn_connection_t* c = pn_event_connection(event);
     pn_connection_set_container(c, app->container_id);
     pn_connection_open(c);
     pn_session_t* s = pn_session(c);
     pn_session_open(s);
     pn_link_t* l = pn_receiver(s, "my_receiver");
     pn_terminus_set_address(pn_link_source(l), NULL);
     pn_link_open(l);
     pn_link_flow(l, 20);
   } break;

   case PN_DELIVERY: {
     /* A message has been received */
     pn_link_t *link = NULL;
     pn_delivery_t *dlv = pn_event_delivery(event);
     if (pn_delivery_readable(dlv) && !pn_delivery_partial(dlv)) {
       link = pn_delivery_link(dlv);
       decode_message(dlv);
       /* Accept the delivery */
       pn_delivery_update(dlv, PN_ACCEPTED);
       /* done with the delivery, move to the next and free it */
       pn_link_advance(link);
       pn_delivery_settle(dlv);  /* dlv is now freed */

//        if (app->message_count == 0) {
//          /* receive forever - see if more credit is needed */
//          if (pn_link_credit(link) < BATCH/2) {
//            /* Grant enough credit to bring it up to BATCH: */
//            pn_link_flow(link, BATCH - pn_link_credit(link));
//          }
//        } else if (++app->received >= app->message_count) {
//          /* done receiving, close the endpoints */
// //          printf("%d messages received\n", app->received);
//          pn_session_t *ssn = pn_link_session(link);
//          pn_link_close(link);
//          pn_session_close(ssn);
//          pn_connection_close(pn_session_connection(ssn));
//        }
     }
   } break;

   case PN_TRANSPORT_ERROR:
    check_condition(event, pn_transport_condition(pn_event_transport(event)));
    pn_connection_close(pn_event_connection(event));
    break;

   case PN_CONNECTION_REMOTE_CLOSE:
    check_condition(event, pn_connection_remote_condition(pn_event_connection(event)));
    pn_connection_close(pn_event_connection(event));
    break;

   case PN_SESSION_REMOTE_CLOSE:
    check_condition(event, pn_session_remote_condition(pn_event_session(event)));
    pn_connection_close(pn_event_connection(event));
    break;

   case PN_LINK_REMOTE_CLOSE:
   case PN_LINK_REMOTE_DETACH:
    check_condition(event, pn_link_remote_condition(pn_event_link(event)));
    pn_connection_close(pn_event_connection(event));
    break;

   default: break;
  }
}

static void check_condition(pn_event_t *e, pn_condition_t *cond) {
  if (VERBOSE) printf("beginning check_condition\n");
  if (pn_condition_is_set(cond)) {
    if (VERBOSE || ERRORS) fprintf(stderr, "%s: %s: %s\n", pn_event_type_name(pn_event_type(e)),
            pn_condition_get_name(cond), pn_condition_get_description(cond));
  }
}

static void decode_message(pn_delivery_t *dlv) {
  static char buffer[MAX_SIZE];
  ssize_t len;
  // try to decode the message body
  if (pn_delivery_pending(dlv) < MAX_SIZE) {
    // read in the raw data
    len = pn_link_recv(pn_delivery_link(dlv), buffer, MAX_SIZE);
    if (len > 0) {
      // decode it into a proton message
      pn_message_t *m = pn_message();
      if (PN_OK == pn_message_decode(m, buffer, len)) {
        pn_string_t *s = pn_string(NULL);
        pn_inspect(pn_message_body(m), s);
        printf("%s\n", pn_string_get(s));
        pn_free(s);
      }
      pn_message_free(m);
    }
  }
}

// reads up to size bytes from data,
// updates data pointer and size to the unread portion of original data,
// returns new value of size
size_t fcd_read(pn_connection_driver_t *driver, uint8_t **data, size_t *size) {
    pn_rwbytes_t buf = pn_connection_driver_read_buffer(driver);
    size_t s = (*size < buf.size) ? *size : buf.size;
    if (buf.start == NULL) {
        exit(1);
    }
    memcpy(buf.start, *data, s);
    
    pn_connection_driver_read_done(driver, s);
    *data += s;
    *size -= s;
    
    return *size;
}

// drops the data
static void fdc_write(pn_connection_driver_t *driver) {
    pn_bytes_t buffer = pn_connection_driver_write_buffer(driver);
    pn_connection_driver_write_done(driver, buffer.size);
}
