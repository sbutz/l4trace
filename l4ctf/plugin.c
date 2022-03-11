#include "../fiasco/ktrace_events.h"
#include <assert.h>
#include <babeltrace2/babeltrace.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define BUF_SIZE 15

/* component's private data */
struct l4trace_in {
    /* Input file path */
    const bt_value *path_value;

    /* Input file */
    FILE *file;

    /* Number of cpu's and streams */
    uint8_t n_cpu;

    /* Stream and event classes */
    bt_event_class *event_class;
    //TODO: use bt_value_array
    //bt_value *streams;
    bt_stream **streams;
};

/* message iterator's private data */
struct l4trace_in_message_iterator {
    /* link to component's private data */
    struct l4trace_in *l4trace_in;

    l4_tracebuffer_entry_t buffer[BUF_SIZE];
};

static
void determine_number_of_cpus(struct l4trace_in *l4trace_in) {
    /* file should be opened */
    assert(l4trace_in->file != NULL);
    l4_tracebuffer_entry_t buf[BUF_SIZE];
    int n_cpu = 0;
    for (;;) {
        int n_read = fread(&buf, sizeof(l4_tracebuffer_entry_t), BUF_SIZE, l4trace_in->file);
    
        if (n_read == EOF || feof(l4trace_in->file)) {
            break;
        }

        for (int i = 0; i < n_read; i++) {
            n_cpu = buf[i]._cpu > n_cpu ? buf[i]._cpu : n_cpu;
        }
    }
    /* cpu numbering starts with zero */
    l4trace_in->n_cpu = n_cpu + 1;

    /* rewind file */
    rewind(l4trace_in->file);
}

static
void create_metadata_and_stream(bt_self_component *self_component,
                           struct l4trace_in *l4trace_in)
{
    /* Create trace */
    bt_trace_class *trace_class = bt_trace_class_create(self_component);

    /* Create stream class within the trace */
    bt_stream_class *stream_class = bt_stream_class_create(trace_class);

    /* Disable automatic stream id */
    bt_stream_class_set_assigns_automatic_stream_id(stream_class, BT_FALSE);

    /* Create clock for the stream */
    bt_clock_class *clock_class = bt_clock_class_create(self_component);
    bt_clock_class_set_frequency(clock_class, 1000000);
    bt_stream_class_set_default_clock_class(stream_class, clock_class);

    /* Create event classes within the stream */
    bt_event_class *event_class = bt_event_class_create(stream_class);
    bt_event_class_set_name(event_class, "l4_trace_record");

    /* Define event structure */
    bt_field_class *payload_field_class =
            bt_field_class_structure_create(trace_class);

    bt_field_class *number_field_class =
            bt_field_class_integer_unsigned_create(trace_class);
    bt_field_class_structure_append_member(payload_field_class, "number",
                                           number_field_class);
    bt_field_class *ip_field_class =
            bt_field_class_integer_unsigned_create(trace_class);
    bt_field_class_structure_append_member(payload_field_class, "ip",
                                           ip_field_class);
    bt_field_class *tsc_field_class =
            bt_field_class_integer_unsigned_create(trace_class);
    bt_field_class_structure_append_member(payload_field_class, "tsc",
                                           tsc_field_class);
    bt_field_class *kclock_field_class =
            bt_field_class_integer_unsigned_create(trace_class);
    bt_field_class_structure_append_member(payload_field_class, "kclock",
                                           kclock_field_class);
    bt_field_class *type_field_class =
            bt_field_class_integer_unsigned_create(trace_class);
    bt_field_class_structure_append_member(payload_field_class, "type",
                                           type_field_class);
    bt_field_class *cpu_field_class =
            bt_field_class_integer_unsigned_create(trace_class);
    bt_field_class_structure_append_member(payload_field_class, "cpu",
                                           cpu_field_class);

    bt_event_class_set_payload_field_class(event_class, payload_field_class);
    l4trace_in->event_class = event_class;


    /* Create the trace */
    bt_trace *trace = bt_trace_create(trace_class);
    /* Create on stream per cpu in the trace */
    l4trace_in->streams = malloc(l4trace_in->n_cpu * sizeof(bt_stream *));
    for (int i = 0; i < l4trace_in->n_cpu; i++) {
        l4trace_in->streams[i] =
            bt_stream_create_with_id(stream_class, trace, i);
    }

    /* Free unneeded references */
    bt_field_class_put_ref(payload_field_class);
    bt_field_class_put_ref(number_field_class);
    bt_field_class_put_ref(ip_field_class);
    bt_field_class_put_ref(tsc_field_class);
    bt_field_class_put_ref(kclock_field_class);
    bt_field_class_put_ref(type_field_class);
    bt_field_class_put_ref(cpu_field_class);
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
    struct l4trace_in *l4trace_in = malloc(sizeof(struct l4trace_in));
    if (!l4trace_in)
        return BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_MEMORY_ERROR;

    l4trace_in->path_value =
        bt_value_map_borrow_entry_value_const(params, "path");
    bt_value_get_ref(l4trace_in->path_value);
 

    /* save file handle */
    l4trace_in->file = fopen(
        bt_value_string_get(l4trace_in->path_value), "r");
    if (!l4trace_in->file) {
        free(l4trace_in);
        return BT_COMPONENT_CLASS_INITIALIZE_METHOD_STATUS_ERROR;
    }

    /* determine number of cpus */
    determine_number_of_cpus(l4trace_in);

    /* casting */
    bt_self_component *self_component =
            bt_self_component_source_as_self_component(self_component_source);

    /* Create metadata, stream and event classes */
    create_metadata_and_stream(self_component, l4trace_in);

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
    struct l4trace_in *l4trace_in = bt_self_component_get_data(
        bt_self_component_source_as_self_component(self_component_source));

    /* Free file path and file */
    bt_value_put_ref(l4trace_in->path_value);
    fclose(l4trace_in->file);

    /* Free events and streams */
    bt_event_class_put_ref(l4trace_in->event_class);
    for (int i = 0; i < l4trace_in->n_cpu; i++)
        bt_stream_put_ref(l4trace_in->streams[i]);
    free(l4trace_in->streams);

    /* Free private data */
    free(l4trace_in);
}

static
bt_message_iterator_class_initialize_method_status
l4trace_in_message_iterator_initialize(
        bt_self_message_iterator *self_message_iterator,
        bt_self_message_iterator_configuration *configuration,
        bt_self_component_port_output *self_port)
{
    /* init iterators private data */
    struct l4trace_in_message_iterator *private =
            malloc(sizeof(struct l4trace_in_message_iterator));
    if (!private)
        return BT_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD_STATUS_MEMORY_ERROR;

    /* save link to component's private data */
    private->l4trace_in = bt_self_component_get_data(
            bt_self_message_iterator_borrow_component(self_message_iterator));

    bt_self_message_iterator_set_data(self_message_iterator, private);
    return BT_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD_STATUS_OK;
}

static
void l4trace_in_message_iterator_finalize(
        bt_self_message_iterator *self_message_iterator)
{
    struct l4trace_in_message_iterator *private =
            bt_self_message_iterator_get_data(self_message_iterator);

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

    *count = 0;

    int n_read = fread(private->buffer, sizeof(l4_tracebuffer_entry_t),
                        BUF_SIZE, private->l4trace_in->file);
    if (n_read == EOF || feof(private->l4trace_in->file)) {
        for (int i = 0; i < private->l4trace_in->n_cpu; i++, (*count)++)
            messages[*count] = bt_message_stream_end_create(
                self_message_iterator, private->l4trace_in->streams[i]);
        return BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_END;
    }

    for (int i = 0; i < BUF_SIZE; i++) {
        l4_tracebuffer_entry_t *e = &private->buffer[i];
        bt_message *message = bt_message_event_create_with_default_clock_snapshot(
                self_message_iterator,
                private->l4trace_in->event_class,
                private->l4trace_in->streams[e->_cpu],
                private->buffer[i]._kclock);
        bt_event *event = bt_message_event_borrow_event(message);
        bt_field *payload_field = bt_event_borrow_payload_field(event);
        bt_field *number_field = bt_field_structure_borrow_member_field_by_name(
                payload_field, "number");
        bt_field_integer_unsigned_set_value(number_field, e->_number);
        bt_field *ip_field = bt_field_structure_borrow_member_field_by_name(
                payload_field, "ip");
        bt_field_integer_unsigned_set_value(ip_field, e->_ip);
        bt_field *tsc_field = bt_field_structure_borrow_member_field_by_name(
                payload_field, "tsc");
        bt_field_integer_unsigned_set_value(tsc_field, e->_tsc);
        bt_field *klock_field = bt_field_structure_borrow_member_field_by_name(
                payload_field, "kclock");
        bt_field_integer_unsigned_set_value(klock_field, e->_kclock);
        bt_field *type_field = bt_field_structure_borrow_member_field_by_name(
                payload_field, "type");
        bt_field_integer_unsigned_set_value(type_field, e->_type);
        bt_field *cpu_field = bt_field_structure_borrow_member_field_by_name(
                payload_field, "cpu");
        bt_field_integer_unsigned_set_value(cpu_field, e->_cpu);
        messages[i] = message;
        *count = i;
    }

    return BT_MESSAGE_ITERATOR_CLASS_NEXT_METHOD_STATUS_OK;
}

BT_PLUGIN_MODULE();

BT_PLUGIN(l4trace);

BT_PLUGIN_SOURCE_COMPONENT_CLASS(input, l4trace_in_message_iterator_next);

BT_PLUGIN_SOURCE_COMPONENT_CLASS_INITIALIZE_METHOD(input, l4trace_in_initialize);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_FINALIZE_METHOD(input, l4trace_in_finalize);

BT_PLUGIN_SOURCE_COMPONENT_CLASS_MESSAGE_ITERATOR_CLASS_INITIALIZE_METHOD(input, l4trace_in_message_iterator_initialize);
BT_PLUGIN_SOURCE_COMPONENT_CLASS_MESSAGE_ITERATOR_CLASS_FINALIZE_METHOD(input, l4trace_in_message_iterator_finalize);
