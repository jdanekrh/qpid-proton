/*
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 */
#include <unistd.h>

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "proton/connection_driver.h"
#include "proton/engine.h"
#include "proton/log.h"
#include "proton/message.h"

#include <benchmark/benchmark.h>
#include <proton/listener.h>
#include <proton/netaddr.h>
#include <proton/proactor.h>
#include <proton/sasl.h>
#include <wait.h>

// This fuzzer is a variant of the receive.c proactor example

#define MAX_SIZE 1024

typedef char str[MAX_SIZE];

typedef struct app_data_t {
  str container_id;
  pn_rwbytes_t message_buffer;
  int message_count = 50 * 1000;
  int received;
  pn_message_t *message;
  int sent = 0;
  pn_rwbytes_t msgout;
  int credit_window = 5000;
  int acknowledged;
  int closed = 0;
} app_data_t;

static void fdc_write(pn_connection_driver_t *driver);

size_t fcd_read(pn_connection_driver_t *driver, uint8_t **data, size_t *size);

static void decode_message(pn_delivery_t *dlv);

// static void handle(app_data_t *app, pn_event_t *event);

static void check_condition(pn_event_t *e, pn_condition_t *cond);

int directmain();

void shovel(pn_connection_driver_t &sender, pn_connection_driver_t &receiver);

// I could not get rid of the error messages on stderr in any other way
void devnull(pn_transport_t *transport, const char *message) {}

static void handle_receiver(app_data_t *app, pn_event_t *event);

static void handle_sender(app_data_t *app, pn_event_t *event);

// stdout
void stdoutlogger(const char *message) { printf("%s\n", message); }

//const bool VERBOSE = true;
const bool VERBOSE = false;
const bool ERRORS = true;
// const bool ERRORS = false;

// static void BM_SendReceiveMessages() {
//    directmain();
//}

// typedef struct app_data_t {
//    const char *host, *port;
//    const char *amqp_address;
//    const char *container_id;
//    int message_count;
//
//    pn_proactor_t *proactor;
//    pn_listener_t *listener;
//    pn_rwbytes_t msgin, msgout;   /* Buffers for incoming/outgoing messages */
//
//    /* Sender values */
//    int sent;
//    int acknowledged;
//    pn_link_t *sender;
//
//    /* Receiver values */
//    int received;
//} app_data_t;

static const int BATCH = 1000; /* Batch size for unlimited receive */

static int exit_code = 0;

/* Close the connection and the listener so so we will get a
 * PN_PROACTOR_INACTIVE event and exit, once all outstanding events
 * are processed.
 */
// static void close_all(pn_connection_t *c, app_data_t *app) {
//    if (c) pn_connection_close(c);
//    if (app->listener) pn_listener_close(app->listener);
//}
//
// static void check_condition(pn_event_t *e, pn_condition_t *cond, app_data_t
// *app) {
//    if (pn_condition_is_set(cond)) {
//        fprintf(stderr, "%s: %s: %s\n", pn_event_type_name(pn_event_type(e)),
//                pn_condition_get_name(cond),
//                pn_condition_get_description(cond));
//        close_all(pn_event_connection(e), app);
//        exit_code = 1;
//    }
//}

///* Create a message with a map { "sequence" : number } encode it and return
///the encoded buffer. */
// static void send_message(app_data_t *app, pn_link_t *sender) {
//    /* Construct a message with the map { "sequence": app.sent } */
//    pn_message_t* message = pn_message();
//    pn_data_t* body = pn_message_body(message);
//    pn_data_put_int(pn_message_id(message), app->sent); /* Set the message_id
//    also */ pn_data_put_map(body); pn_data_enter(body);
//    pn_data_put_string(body, pn_bytes(sizeof("sequence")-1, "sequence"));
//    pn_data_put_int(body, app->sent); /* The sequence number */
//    pn_data_exit(body);
//    if (pn_message_send(message, sender, &app->msgout) < 0) {
//        fprintf(stderr, "send error: %s\n",
//        pn_error_text(pn_message_error(message))); exit_code = 1;
//    }
//    pn_message_free(message);
//}

static void decode_message(pn_rwbytes_t data) {
  pn_message_t *m = pn_message();
  int err = pn_message_decode(m, data.start, data.size);
  if (!err) {
    /* Print the decoded message */
    pn_string_t *s = pn_string(NULL);
    pn_inspect(pn_message_body(m), s);
    printf("%s\n", pn_string_get(s));
    fflush(stdout);
    pn_free(s);
    pn_message_free(m);
    free(data.start);
  } else {
    fprintf(stderr, "decode error: %s\n", pn_error_text(pn_message_error(m)));
    exit_code = 1;
  }
}

///* This function handles events when we are acting as the receiver */
// static void handle_receive(app_data_t *app, pn_event_t* event) {
//    switch (pn_event_type(event)) {
//
//        case PN_LINK_REMOTE_OPEN: {
//            pn_link_t *l = pn_event_link(event);
//            pn_link_open(l);
//            pn_link_flow(l, app->message_count ? app->message_count : BATCH);
//        } break;
//
//        case PN_DELIVERY: {          /* Incoming message data */
//            pn_delivery_t *d = pn_event_delivery(event);
//            if (pn_delivery_readable(d)) {
//                pn_link_t *l = pn_delivery_link(d);
//                size_t size = pn_delivery_pending(d);
//                pn_rwbytes_t* m = &app->msgin; /* Append data to incoming
//                message buffer */ ssize_t recv; m->size += size; m->start =
//                (char*)realloc(m->start, m->size); recv = pn_link_recv(l,
//                m->start, m->size); if (recv == PN_ABORTED) {
//                    printf("Message aborted\n");
//                    fflush(stdout);
//                    m->size = 0;           /* Forget the data we accumulated
//                    */ pn_delivery_settle(d); /* Free the delivery so we can
//                    receive the next message */ pn_link_flow(l, 1);    /*
//                    Replace credit for aborted message */
//                } else if (recv < 0 && recv != PN_EOS) {        /* Unexpected
//                error */
//                    pn_condition_format(pn_link_condition(l), "broker",
//                    "PN_DELIVERY error: %s", pn_code(recv)); pn_link_close(l);
//                    /* Unexpected error, close the link */
//                } else if (!pn_delivery_partial(d)) { /* Message is complete
//                */
//                    decode_message(*m);
//                    *m = pn_rwbytes_null;
//                    pn_delivery_update(d, PN_ACCEPTED);
//                    pn_delivery_settle(d);  /* settle and free d */
//                    if (app->message_count == 0) {
//                        /* receive forever - see if more credit is needed */
//                        if (pn_link_credit(l) < BATCH/2) {
//                            pn_link_flow(l, BATCH - pn_link_credit(l));
//                        }
//                    } else if (++app->received >= app->message_count) {
//                        printf("%d messages received\n", app->received);
//                        close_all(pn_event_connection(event), app);
//                    }
//                }
//            }
//            break;
//        }
//        default:
//            break;
//    }
//}
//
///* This function handles events when we are acting as the sender */
// static void handle_send(app_data_t* app, pn_event_t* event) {
//    switch (pn_event_type(event)) {
//
//        case PN_LINK_REMOTE_OPEN: {
//            pn_link_t* l = pn_event_link(event);
//            pn_terminus_set_address(pn_link_target(l), app->amqp_address);
//            pn_link_open(l);
//        } break;
//
//        case PN_LINK_FLOW: {
//            /* The peer has given us some credit, now we can send messages */
//            pn_link_t *sender = pn_event_link(event);
//            while (pn_link_credit(sender) > 0 && app->sent <
//            app->message_count) {
//                ++app->sent;
//                /* Use sent counter as unique delivery tag. */
//                pn_delivery(sender, pn_dtag((const char *)&app->sent,
//                sizeof(app->sent))); send_message(app, sender);
//            }
//            break;
//        }
//
//        case PN_DELIVERY: {
//            /* We received acknowledgement from the peer that a message was
//            delivered. */ pn_delivery_t* d = pn_event_delivery(event); if
//            (pn_delivery_remote_state(d) == PN_ACCEPTED) {
//                if (++app->acknowledged == app->message_count) {
//                    printf("%d messages sent and acknowledged\n",
//                    app->acknowledged); close_all(pn_event_connection(event),
//                    app);
//                }
//            }
//        } break;
//
//        default:
//            break;
//    }
//}
//
///* Handle all events, delegate to handle_send or handle_receive depending on
///link mode.
//   Return true to continue, false to exit
//*/
// static bool handle(app_data_t* app, pn_event_t* event) {
//    switch (pn_event_type(event)) {
//
//        case PN_LISTENER_OPEN: {
//            printf("PN_LISTENER_OPEN\n");
//            char port[256];             /* Get the listening port */
//            pn_netaddr_host_port(pn_listener_addr(pn_event_listener(event)),
//            NULL, 0, port, sizeof(port)); printf("listening on %s\n", port);
//            fflush(stdout);
//            break;
//        }
//        case PN_LISTENER_ACCEPT:
//            printf("PN_LISTENER_ACCEPT\n");
//            pn_listener_accept2(pn_event_listener(event), NULL, NULL);
//            break;
//
//        case PN_CONNECTION_INIT:
//            printf("PN_CONNECTION_INIT\n");
//            pn_connection_set_container(pn_event_connection(event),
//            app->container_id); break;
//
//        case PN_CONNECTION_BOUND: {
//            printf("PN_CONNECTION_BOUND\n");
//            /* Turn off security */
//            pn_transport_t *t = pn_event_transport(event);
//            pn_transport_require_auth(t, false);
//            pn_sasl_allowed_mechs(pn_sasl(t), "ANONYMOUS");
//            break;
//        }
//        case PN_CONNECTION_REMOTE_OPEN: {
//            printf("PN_CONNECTION_REMOTE_OPEN\n");
//            pn_connection_open(pn_event_connection(event)); /* Complete the
//            open */ break;
//        }
//
//        case PN_SESSION_REMOTE_OPEN: {
//            printf("PN_SESSION_REMOTE_OPEN\n");
//            pn_session_open(pn_event_session(event));
//            break;
//        }
//
//        case PN_TRANSPORT_CLOSED:
//            printf("PN_TRANSPORT_CLOSED\n");
//            check_condition(event,
//            pn_transport_condition(pn_event_transport(event)), app); break;
//
//        case PN_CONNECTION_REMOTE_CLOSE:
//            printf("PN_CONNECTION_REMOTE_CLOSE\n");
//            check_condition(event,
//            pn_connection_remote_condition(pn_event_connection(event)), app);
//            pn_connection_close(pn_event_connection(event)); /* Return the
//            close */ break;
//
//        case PN_SESSION_REMOTE_CLOSE:
//            printf("PN_SESSION_REMOTE_CLOSE\n");
//            check_condition(event,
//            pn_session_remote_condition(pn_event_session(event)), app);
//            pn_session_close(pn_event_session(event)); /* Return the close */
//            pn_session_free(pn_event_session(event));
//            break;
//
//        case PN_LINK_REMOTE_CLOSE:
//        case PN_LINK_REMOTE_DETACH:
//            printf("PN_LINK_REMOTE_DETACH\n");
//            check_condition(event,
//            pn_link_remote_condition(pn_event_link(event)), app);
//            pn_link_close(pn_event_link(event)); /* Return the close */
//            pn_link_free(pn_event_link(event));
//            break;
//
//        case PN_PROACTOR_TIMEOUT:
//            printf("PN_PROACTOR_TIMEOUT\n");
//            /* Wake the sender's connection */
//            pn_connection_wake(pn_session_connection(pn_link_session(app->sender)));
//            break;
//
//        case PN_LISTENER_CLOSE:
//            printf("PN_LISTENER_CLOSE\n");
//            app->listener = NULL;        /* Listener is closed */
//            check_condition(event,
//            pn_listener_condition(pn_event_listener(event)), app); break;
//
//        case PN_PROACTOR_INACTIVE:
//            printf("PN_PROACTOR_INACTIVE\n");
//            return false;
//            break;
//
//        default: {
//            pn_link_t *l = pn_event_link(event);
//            if (l) {                      /* Only delegate link-related events
//            */
//                if (pn_link_is_sender(l)) {
//                    handle_send(app, event);
//                } else {
//                    handle_receive(app, event);
//                }
//            }
//        }
//    }
//    return exit_code == 0;
//}
//
// void run(app_data_t *app) {
//    /* Loop and handle events */
//    do {
//        pn_event_batch_t *events = pn_proactor_wait(app->proactor);
//        pn_event_t *e;
//        for (e = pn_event_batch_next(events); e; e =
//        pn_event_batch_next(events)) {
//            if (!handle(app, e)) {
//                return;
//            }
//        }
//        pn_proactor_done(app->proactor, events);
//    } while(true);
//}
//
// int directmain() {
//    struct app_data_t app = {0};
//    char addr[PN_MAX_ADDR];
//    app.container_id = "baf";
//    app.host = "localhost";
//    app.port = "amqp";
//    app.amqp_address = "examples";
//    app.message_count = 10;
//
//    /* Create the proactor and connect */
//    app.proactor = pn_proactor();
//    app.listener = pn_listener();
//    pn_proactor_addr(addr, sizeof(addr), app.host, app.port);
//    pn_proactor_listen(app.proactor, app.listener, addr, 16);
////    pn_proactor_clisten(app.proactor, app.listener, addr, 16);
//    pn_proactor_connect2(app.proactor, NULL, NULL, "");
//    run(&app);
//    pn_proactor_free(app.proactor);
//    free(app.msgout.start);
//    free(app.msgin.start);
//    return exit_code;
//}

static void BM_SendReceiveMessages(benchmark::State &state) {
  if (VERBOSE)
    printf("BEGIN BM_SendReceiveMessages\n");

  for (auto _ : state) {
    //        app.closed = false;
    app_data_t app;
    app.message = pn_message();
    sprintf(app.container_id, "%s:%06x", "BM_SendReceiveMessages",
            rand() & 0xffffff);

    pn_connection_driver_t receiver;
    if (pn_connection_driver_init(&receiver, NULL, NULL) != 0) {
      printf("receiver: pn_connection_driver_init failed\n");
      exit(1);
    }

    pn_connection_driver_t sender;
    if (pn_connection_driver_init(&sender, NULL, NULL) != 0) {
      printf("sender: pn_connection_driver_init failed\n");
      exit(1);
    }

    //            pn_log_enable(true);
    //            pn_log_logger(stdoutlogger);
    //            pn_transport_trace(receiver.transport, PN_TRACE_FRM |
    //            PN_TRACE_EVT); pn_transport_trace(sender.transport,
    //            PN_TRACE_FRM | PN_TRACE_EVT);

    //    pn_transport_trace(receiver.transport, PN_TRACE_EVT);
    //    pn_transport_trace(sender.transport, PN_TRACE_EVT);
    //    pn_log_enable(false);
    //    pn_log_logger(NULL);
    //    pn_transport_trace(receiver.transport, PN_TRACE_OFF);
    //    pn_transport_set_tracer(receiver.transport, devnull);
    //
    //    pn_transport_trace(sender.transport, PN_TRACE_OFF);
    //    pn_transport_set_tracer(sender.transport, devnull);

    //            printf("aaaa\n");
    //        sleep(1);
    do {
      pn_event_t *event;
      while ((event = pn_connection_driver_next_event(&sender)) != NULL) {
        handle_sender(&app, event);
      }
      shovel(sender, receiver);
      while ((event = pn_connection_driver_next_event(&receiver)) != NULL) {
        handle_receiver(&app, event);
      }
      shovel(receiver, sender);
    } while (app.closed < 2);

    pn_connection_driver_close(&receiver);
    pn_connection_driver_close(&sender);

    shovel(receiver, sender);
    shovel(sender, receiver);

    // this can take long time, up to 500 ms
    //        pn_connection_driver_destroy(&receiver);
    //        pn_connection_driver_destroy(&sender);
  }

  if (VERBOSE)
    printf("END BM_SendReceiveMessages\n");
}

void shovel(pn_connection_driver_t &sender, pn_connection_driver_t &receiver) {
  pn_bytes_t wbuf = pn_connection_driver_write_buffer(&receiver);
  if (wbuf.size == 0) {
    pn_connection_driver_write_done(&receiver, 0);
    return;
  }

  pn_rwbytes_t rbuf = pn_connection_driver_read_buffer(&sender);
  if (rbuf.start == NULL) {
    printf("shovel: rbuf.start is null\n");
    fflush(stdout);
    fflush(stderr);
    exit(1);
  }

  size_t s = rbuf.size < wbuf.size ? rbuf.size : wbuf.size;
  memcpy(rbuf.start, wbuf.start, s);

  pn_connection_driver_read_done(&sender, s);
  pn_connection_driver_write_done(&receiver, s);
}

BENCHMARK(BM_SendReceiveMessages)->Unit(benchmark::kMillisecond);

///* Create a message with a map { "sequence" : number } encode it and return
///the encoded buffer. */
static void send_message(app_data_t *app, pn_link_t *sender) {
  /* Construct a message with the map { "sequence": app.sent } */
  pn_data_t *body;
  pn_message_clear(app->message);
  body = pn_message_body(app->message);
  pn_data_put_int(pn_message_id(app->message),
                  app->sent); /* Set the message_id also */
  pn_data_put_map(body);
  pn_data_enter(body);
  pn_data_put_string(body, pn_bytes(sizeof("sequence") - 1, "sequence"));
  pn_data_put_int(body, 42); /* The sequence number */
  pn_data_exit(body);
  if (pn_message_send(app->message, sender, &app->message_buffer) < 0) {
    fprintf(stderr, "error sending message: %s\n",
            pn_error_text(pn_message_error(app->message)));
    exit(1);
  }
}

static void handle_sender(app_data_t *app, pn_event_t *event) {
  switch (pn_event_type(event)) {

  case PN_CONNECTION_INIT: {
    pn_connection_t *c = pn_event_connection(event);
    pn_connection_set_container(c, "sendercid");
    pn_connection_open(c);
    pn_session_t *s = pn_session(c);
    pn_session_open(s);

    pn_link_t *l = pn_sender(s, "my_sender");
    pn_terminus_set_address(pn_link_target(l), "example");
    pn_link_set_snd_settle_mode(l, PN_SND_UNSETTLED);
    pn_link_set_rcv_settle_mode(l, PN_RCV_FIRST);
    pn_link_open(l);
  } break;

  case PN_LINK_FLOW: {
    //            printf("got flow sender");
    /* The peer has given us some credit, now we can send messages */
    pn_link_t *sender = pn_event_link(event);
    while (pn_link_credit(sender) > 0 /*&& app->sent < app->message_count*/) {
      ++app->sent;
      /* Use sent counter as unique delivery tag. */
      pn_delivery(sender, pn_dtag((const char *)&app->sent, sizeof(app->sent)));
      send_message(app, sender);
    }
    break;
  }

  case PN_DELIVERY: {
    /* We received acknowledgement from the peer that a message was delivered.
     */
    pn_delivery_t *d = pn_event_delivery(event);
    if (pn_delivery_remote_state(d) == PN_ACCEPTED) {
      //                app->accepted++;
      //                printf("got PN_ACCEPTED\n");
      if (++app->acknowledged == app->message_count) {
        //                    printf("%d messages sent and acknowledged\n",
        //                    app->acknowledged);
        pn_connection_close(pn_event_connection(event));
        /* Continue handling events till we receive TRANSPORT_CLOSED */
      }
    } else {
      fprintf(stderr, "unexpected delivery state %d\n",
              (int)pn_delivery_remote_state(d));
      pn_connection_close(pn_event_connection(event));
      //                exit_code=1;
    }
    break;
  }

  case PN_CONNECTION_REMOTE_OPEN:
    pn_connection_open(pn_event_connection(event)); /* Complete the open */
    break;

  case PN_SESSION_REMOTE_OPEN:
    pn_session_open(pn_event_session(event));
    break;

  case PN_TRANSPORT_ERROR:
    check_condition(event, pn_transport_condition(pn_event_transport(event)));
    pn_connection_close(pn_event_connection(event));
    break;

  case PN_CONNECTION_REMOTE_CLOSE:
    check_condition(event,
                    pn_connection_remote_condition(pn_event_connection(event)));
    pn_connection_close(pn_event_connection(event));
    break;

  case PN_SESSION_REMOTE_CLOSE:
    check_condition(event,
                    pn_session_remote_condition(pn_event_session(event)));
    pn_connection_close(pn_event_connection(event));
    break;

  case PN_LINK_REMOTE_CLOSE:
  case PN_LINK_REMOTE_DETACH:
    check_condition(event, pn_link_remote_condition(pn_event_link(event)));
    pn_connection_close(pn_event_connection(event));
    break;

  case PN_TRANSPORT_CLOSED:
    app->closed++;
    break;

  default:
    break;
  }
}

static void handle_receiver(app_data_t *app, pn_event_t *event) {
  switch (pn_event_type(event)) {
    //        case PN_LINK_FLOW: {
    //            printf("got flow receiver");
    //            fflush(stdout);
    //            /* The peer has given us some credit, now we can send messages
    //            */ pn_link_t *sender = pn_event_link(event); while
    //            (pn_link_credit(sender) > 0 /*&& app->sent <
    //            app->message_count*/) {
    ////                ++app->sent;
    //                /* Use sent counter as unique delivery tag. */
    //                pn_delivery(sender, pn_dtag((const char *)&app->sent,
    //                sizeof(app->sent))); send_message(app, sender);
    //            }
    //            break;
    //        }

  case PN_CONNECTION_INIT: {
    pn_connection_t *c = pn_event_connection(event);
    pn_connection_set_container(c, app->container_id);
    //            pn_connection_open(c);
    pn_session_t *s = pn_session(c);
    pn_session_open(s);

    pn_link_t *l = pn_receiver(s, "my_receiver");
    pn_terminus_set_address(pn_link_source(l), "example");
    pn_link_open(l);
  } break;

  case PN_LINK_REMOTE_OPEN: {
    pn_link_t *l = pn_event_link(event);
    pn_terminus_t *t = pn_link_target(l);
    pn_terminus_t *rt = pn_link_remote_target(l);
    pn_terminus_set_address(t, pn_terminus_get_address(rt));
    pn_link_open(l);
    if (pn_link_is_receiver(l)) {
      pn_link_flow(l, app->credit_window);
    }
    break;
  }

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
      pn_delivery_settle(dlv); /* dlv is now freed */
    }
    pn_link_flow(link, app->credit_window - pn_link_credit(link));
  } break;

  case PN_CONNECTION_REMOTE_OPEN:
    pn_connection_open(pn_event_connection(event)); /* Complete the open */
    break;

  case PN_SESSION_REMOTE_OPEN:
    pn_session_open(pn_event_session(event));
    break;

  case PN_TRANSPORT_ERROR:
    check_condition(event, pn_transport_condition(pn_event_transport(event)));
    pn_connection_close(pn_event_connection(event));
    break;

  case PN_CONNECTION_REMOTE_CLOSE:
    check_condition(event,
                    pn_connection_remote_condition(pn_event_connection(event)));
    pn_connection_close(pn_event_connection(event));
    break;

  case PN_SESSION_REMOTE_CLOSE:
    check_condition(event,
                    pn_session_remote_condition(pn_event_session(event)));
    pn_connection_close(pn_event_connection(event));
    break;

  case PN_LINK_REMOTE_CLOSE:
  case PN_LINK_REMOTE_DETACH:
    check_condition(event, pn_link_remote_condition(pn_event_link(event)));
    pn_connection_close(pn_event_connection(event));
    break;

  case PN_TRANSPORT_CLOSED:
    app->closed++;
    break;

  default:
    break;
  }
}

//
static void check_condition(pn_event_t *e, pn_condition_t *cond) {
  if (VERBOSE)
    printf("beginning check_condition\n");
  if (pn_condition_is_set(cond)) {
    if (VERBOSE || ERRORS)
      fprintf(stderr, "%s: %s: %s\n", pn_event_type_name(pn_event_type(e)),
              pn_condition_get_name(cond), pn_condition_get_description(cond));
  }
}

//
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
        if (ERRORS)
          //                    printf("%s\n", pn_string_get(s));
          pn_free(s);
      }
      pn_message_free(m);
    }
  }
}
//
//// reads up to `size` bytes from `data`,
//// updates `data` pointer and `size` to the unread portion of original `data`,
//// returns new value of `size`
// size_t fcd_read(pn_connection_driver_t *driver, uint8_t **data, size_t *size)
// {
//    pn_rwbytes_t buf = pn_connection_driver_read_buffer(driver);
//    size_t s = (*size < buf.size) ? *size : buf.size;
//    if (buf.start == NULL) {
//        exit(1);
//    }
//    memcpy(buf.start, *data, s);
//
//    pn_connection_driver_read_done(driver, s);
//    *data += s;
//    *size -= s;
//
//    return *size;
//}
//
//// drops the data in the buffer and reports them as written
// static void fdc_write(pn_connection_driver_t *driver) {
//    pn_bytes_t buffer = pn_connection_driver_write_buffer(driver);
//    pn_connection_driver_write_done(driver, buffer.size);
//}

BENCHMARK_MAIN();

// int main() {
//    benchmark::State *state = nullptr;
//    BM_SendReceiveMessages(*state);
//}
