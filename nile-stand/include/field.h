#ifndef FIELD_H
#define FIELD_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include "uart.h"

/**
 * Enumeration of the possible types that a serializable field (`field_t`) can
 * take on.
 */
typedef enum {
    FIELD_TYPE_UNSIGNED_INT,
    FIELD_TYPE_SIGNED_INT,
    FIELD_TYPE_FLOAT,
    FIELD_TYPE_BOOLEAN
} field_value_type_e;

/**
 * Value of a `field_value_t`, a union of all possible types of data for fields.
 */
typedef union {
    uint64_t unsigned_int;
    int64_t signed_int;
    float floating;
    bool boolean;
} field_value_u;

/**
 * A typed value for serializable fields.
 */
typedef struct {
    field_value_type_e field_type;
    field_value_u field_value;
} field_value_t;

/**
 * A field for serialization, including a name, type, and value.
 */
typedef struct {
    field_value_t value;
    char* name;
} field_t;

/**
 * Update a field of the given c-string `name` to be equal to the given
 * `field_value_t` by sending that name and value (with a type) over serial
 * using the given print function.
 */
void update_field(uart_t* uart, field_t field);

/**
 * The type of a command, what action it represents.
 */
typedef enum {
	COMMAND_OPEN,   // Open a valve
	COMMAND_CLOSE,  // Close a valve
	COMMAND_IGNITE  // Ignite the match
} command_type_e;

/**
 * A specific valve on the stand.
 */
typedef enum {
	NP1, NP2, NP3, NP4, IP1, IP2, IP3, NONE
} valve_e;

/**
 * A command recieved from the operator console over serial.
 */
typedef struct {
	command_type_e cmd_type;
	valve_e cmd_valve;
} command_t;

/**
 * Parses a command, placing the result into the given `comand_t` pointer.
 * Returns `0` on success, and a negative number on failure.
 */
int parse_command(const char* str, command_t* command);

/**
 * An iterator over the lines of a base string.
 */
typedef struct {
	char* base;
	char* pos;
	char* current_line;
} line_iterator_t;

/**
 * Create a new iterator over the lines of the given `base` string.
 */
line_iterator_t make_line_iterator(char* base);

/**
 * Return the next line of the iterator, or `NULL` if exhuasted.
 *
 * The pointer returned by this function is only valid until the next time this
 * function is called on the given `line_iterator_t`. If the data refered to by
 * that pointer must live longer than that, the consuming function should clone
 * the returned string.
 */
char* line_iterator_next(line_iterator_t* iter);

/**
 * A remainder holding iterator over lines of text.
 */
typedef struct {
	line_iterator_t line_iter;
	char* remainder;
} text_stream_t;

/**
 * Make a new `text_stream_t`, `base` may not be `NULL` but may be an empty
 * string. This function clones the memory from `base`, so the given pointer may
 * have its referenced data safely mutated after calling this function.
 */
text_stream_t make_text_stream(const char* base);

/**
 * Get the next line from the `text_stream_t`, or `NULL` if out of characters
 * for now.
 * 
 * The pointer returned by this function is only valid until the next time this
 * function is called on the given `text_stream_t`. If the data refered to by
 * that pointer must live longer than that, the consuming function should clone
 * the returned string.
 */
char* text_stream_next(text_stream_t* stream);

/**
 * Buffer in new text to the `text_stream_t`, allowing for the retrieval of more
 * lines via `text_stream_next`.
 */
void text_stream_buffer(text_stream_t* stream, char* text);

/**
 * Frees the recourses used by the given `text_stream_t`, after calling this
 * function the `text_stream_t` should not be used.
 */
void text_stream_destroy(text_stream_t* stream);

/**
 * Type for reading commands off of a continuous stream of bytes/characters.
 * Even though this type is a `typedef` of `text_stream_t`, the values of either
 * type should not be used interchangeably.
 */
typedef text_stream_t command_reader_t;

/**
 * Creates a new `command_reader_t` with an initial buffer of `buf`. `buf` may
 * be either `NULL` or `""` to indicate an empty starting buffer.
 */
command_reader_t make_command_reader(char* buf);

/**
 * Reads a new command off of the `command_reader_t`'s internal buffer,
 * returning zero on success and a negative value on failure.
 *
 * Unlike similar functions in the module, the data places into the given
 * `command_t` pointer will remain valid indefinetly, and will not be freed by
 * the next call to this function.
 */
int command_reader_read(command_reader_t* reader, command_t* command);

/**
 * Buffer new text into the `command_reader_t`. All data in the buffer is cloned
 * so it can be used freely after calling this function.
 */
void command_reader_buffer(command_reader_t* reader, char* buf);

#endif  /* FIELD_H */
