#include "../fiasco/ktrace_events.h"
#include <babeltrace2/babeltrace.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 1

/*
 * TODO: check function return values
 */

/* component's private data */
struct l4trace_in {
    bt_event_class *event_class;
    bt_stream *stream;
};

/* message iterator's private data */
struct l4trace_in_message_iterator {
    /* link to component's private data */
    struct l4trace_in *l4trace_in;

    //TODO: just for testing
    //TODO: remove
    uint64_t i;

    FILE *file;
    l4_tracebuffer_entry_t buffer[BUF_SIZE];
};

static
void create_metadata_and_stream(bt_self_component *self_component,
                           struct l4trace_in *l4trace_in)
{
    /* Create trace */
    bt_trace_class *trace_class = bt_trace_class_create(self_component);

    /* Create stream within the trace */
    bt_stream_class *stream_class = bt_stream_class_create(trace_class);

    /* Create clock for the stream */
    bt_clock_class *clock_class = bt_clock_class_create(self_component);
    //bt_clock_class_set_frequency(clock_class, 1000000);
    bt_stream_class_set_default_clock_class(stream_class, clock_class);

    /* Create event classes within the stream */
    //TODO: only use if bt_stream_class_assings_automatic_event_class_id == BT_TRUE
    bt_event_class *event_class = bt_event_class_create(stream_class);
    bt_event_class_set_name(event_class, "l4_trace_record");

    /* Define event structure */
    bt_field_class *payload_field_class =
            bt_field_class_structure_create(trace_class);
    bt_field_class *number_field_class =
            bt_field_class_integer_unsigned_create(trace_class);
    bt_field_class_structure_append_member(payload_field_class, "number",
                                           number_field_class);
    bt_field_class *tsc_field_class =
            bt_field_class_integer_unsigned_create(trace_class);
    bt_field_class_structure_append_member(payload_field_class, "tsc",
                                           tsc_field_class);
    bt_event_class_set_payload_field_class(event_class, payload_field_class);
    l4trace_in->event_class = event_class;


    /* Create the component's stream */
    bt_trace *trace = bt_trace_create(trace_class);
    l4trace_in->stream = bt_stream_create(stream_class, trace);

    /* Free unneeded references */
    bt_field_class_put_ref(payload_field_class);
    bt_field_class_put_ref(number_field_class);
    bt_trace_put_ref(trace);
    bt_clock_class_put_ref(clock_class);
    bt_stream_class_put_ref(stream_class);
    bt_trace_class_put_ref(trace_class);
}

static
bt_component_class_initialize_method_status l4trace_in_initialize(
        bt_self_component_source *self_component_source,
        bt_self_component_source_configuration *configuration,
        const bt_value *params,
        void *initialize_method_data
        )
{
    printf("component initialize\n");
    struct l4trace_in *l4trace_in = malloc(sizeof(struct l4trace_in));
    if (!l4trace_in)
        return BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_MEMORY_ERROR;

    /* casting */
    bt_self_component *self_component =
            bt_self_component_source_as_self_component(self_component_source);

    /* Create metadata, stream and event classes */
    create_metadata_and_stream(self_component, l4trace_in);

    printf("packet-support: %x\n", bt_stream_class_supports_packets(bt_stream_borrow_class_const(l4trace_in->stream)) == BT_FALSE);

    printf("clock: %x\n", bt_stream_class_borrow_default_clock_class_const(bt_stream_borrow_class_const(l4trace_in->stream)));

    /* Save component's private data */
    bt_self_component_set_data(self_component, l4trace_in);

    /*
     * Add an output port named `out`.
     * Needed to connect the component to a filter or sink.
     * After another node is connected the message iterator is created.
     */
    bt_self_component_source_add_output_port(self_component_source, "out",
                                             NULL, NULL);

    return BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_OK;
}

static
void l4trace_in_finalize(bt_self_component_source *self_component_source)
{
    printf("finalize component\n");

    struct l4trace_in *l4trace_in = bt_self_component_get_data(
        bt_self_component_source_as_self_component(self_component_source));

    /* Free refrences */
    bt_event_class_put_ref(l4trace_in->event_class);
    bt_stream_put_ref(l4trace_in->stream);

    free(l4trace_in);
}

static
bt_message_iterator_class_initialize_method_status
l4trace_in_message_iterator_initialize(
        bt_self_message_iterator *self_message_iterator,
        bt_self_message_iterator_configuration *configuration,
        bt_self_component_port_output *self_port)
{
    printf("iterator initialize\n");

    /* init iterators private data */
    struct l4trace_in_message_iterator *private =
            malloc(sizeof(struct l4trace_in_message_iterator));
    if (!private)
        return BT_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD_STATUS_MEMORY_ERROR;

    /* save link to component's private data */
    private->l4trace_in = bt_self_component_get_data(
            bt_self_message_iterator_borrow_component(self_message_iterator));

    //TODO: remove
    private->i = 0;

    /* save file handle */
    private->file = fopen("/tmp/l4trace_sorted.out", "r");
    if (!private->file) {
        free(private);
        return BT_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    }

    bt_self_message_iterator_set_data(self_message_iterator, private);
    return BT_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD_STATUS_OK;
}

static
void l4trace_in_message_iterator_finalize(
        bt_self_message_iterator *self_message_iterator)
{
    printf("iterator finalize\n");
    struct l4trace_in_message_iterator *private =
            bt_self_message_iterator_get_data(self_message_iterator);

    fclose(private->file);
    free(private);
}

static
bt_message_iterator_class_next_method_status l4trace_in_message_iterator_next(
        bt_self_message_iterator *self_message_iterator,
        bt_message_array_const messages,
        uint64_t capacity,
        uint64_t *count)
{
    struct l4trace_in_message_iterator *private =
        bt_self_message_iterator_get_data(self_message_iterator);

    int n_read = fread(private->buffer, sizeof(l4_tracebuffer_entry_t),
                        BUF_SIZE, private->file);
    if (n_read == EOF || feof(private->file)) {
        //TODO: create end of stream message
        //message = bt_message_stream_end_create(self_message_iterator, stream?);
        return BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_END;
    }

    /*
     * read from file
     * for each record
     *   drop if not type 2
     *   create message
     *   set type
     */
    *count = 0;
    int i;
    for (i = 0; i < BUF_SIZE; i++) {
        bt_message *message = bt_message_event_create_with_default_clock_snapshot(
                self_message_iterator,
                private->l4trace_in->event_class,
                private->l4trace_in->stream,
                private->buffer[i]._kclock);
        bt_event *event = bt_message_event_borrow_event(message);
        bt_field *payload_field = bt_event_borrow_payload_field(event);
        bt_field *number_field = bt_field_structure_borrow_member_field_by_name(
                payload_field, "number");
        bt_field_integer_unsigned_set_value(number_field,
                                            private->buffer[i]._number);
        bt_field *tsc_field = bt_field_structure_borrow_member_field_by_name(
                payload_field, "tsc");
        bt_field_integer_unsigned_set_value(tsc_field,
                                            private->buffer[i]._kclock);
        messages[i] = message;
    }
    *count = i;

    return BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_OK;
}

BT_PLUGIN_MODULE();

BT_PLUGIN(l4trace);

BT_PLUGIN_SOURCE_COMPONENT_CLASS(input, l4trace_in_message_iterator_next);

BT_PLUGIN_SOURCE_COMPONENT_CLASS_INITIALIZE_METHOD(input, l4trace_in_initialize);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_FINALIZE_METHOD(input, l4trace_in_finalize);

BT_PLUGIN_SOURCE_COMPONENT_CLASS_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD(input, l4trace_in_message_iterator_initialize);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_MESSAGE_ITERATOR_CLASS_FINALIZE_METHOD(input, l4trace_in_message_iterator_finalize);
