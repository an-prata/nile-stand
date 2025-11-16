#include "field.h"
#include "uart.h"

// Replace the `printf` with a call to a function which prints over UART.
//#define SERIAL_PRINT(s) printf("%s", s)
#define SERIAL_PRINT(s) uart_send(s)
#define SERIAL_PRINT_BUFFER_LEN 128

#define COMMAND_MSG_OPEN "OPEN"
#define COMMAND_MSG_CLOSE "CLOSE"
#define COMMAND_MSG_IGNITE "IGNITE"

/**
 * Returns `true` if `prefix` is a prefix of `base`.
 */
static bool is_prefix(const char* base, const char* prefix) {
	return strncmp(base, prefix, strlen(prefix)) == 0;
}

/**
 * Sets the given `valve_e` to the valve corrosponding to the given string,
 * returning zero on success and a negative value on failure.
 */
static valve_e parse_valve(const char* base) {
	if (strcmp(base, "NP1") == 0) {
		return NP1;
	}

	if (strcmp(base, "NP2") == 0) {
		return NP2;
	}

	if (strcmp(base, "NP3") == 0) {
		return NP3;
	}

	if (strcmp(base, "NP4") == 0) {
		return NP4;
	}

	if (strcmp(base, "IP1") == 0) {
		return IP1;
	}

	if (strcmp(base, "IP2") == 0) {
		return IP2;
	}

	if (strcmp(base, "IP3") == 0) {
		return IP3;
	}

	return NONE;
}

void update_field(field_t field) {
    char buf[SERIAL_PRINT_BUFFER_LEN] = { 0 };

    switch (field.value.field_type) {
        case FIELD_TYPE_UNSIGNED_INT:
            snprintf(
                buf,
                SERIAL_PRINT_BUFFER_LEN,
                "%s:u=%llu\n",
                field.name,
                field.value.field_value.unsigned_int
            );

            break;

        case FIELD_TYPE_SIGNED_INT:
            snprintf(
                buf,
                SERIAL_PRINT_BUFFER_LEN,
                "%s:i=%lli\n",
                field.name,
                field.value.field_value.signed_int
            );

            break;

        case FIELD_TYPE_FLOAT:
            snprintf(
                buf,
                SERIAL_PRINT_BUFFER_LEN,
                "%s:f=%f\n",
                field.name,
                field.value.field_value.floating
            );

            break;

        case FIELD_TYPE_BOOLEAN:
            if (field.value.field_value.boolean) {
                snprintf(
                    buf,
                    SERIAL_PRINT_BUFFER_LEN,
                    "%s:b=TRUE\n",
                    field.name
                );
            } else {
                snprintf(
                    buf,
                    SERIAL_PRINT_BUFFER_LEN,
                    "%s:b=FALSE\n",
                    field.name
                );
            }

            break;
            
        default:
            return;
    }

    SERIAL_PRINT(buf);
}

int parse_command(const char* str, command_t* command) {
	if (is_prefix(str, COMMAND_MSG_OPEN)) {
		if (str[strlen(COMMAND_MSG_OPEN)] != ':') {
			return -1;
		}

		if (strlen(&str[strlen(COMMAND_MSG_OPEN) + 1]) != 3) {
			return -1;
		}

		command->cmd_type = COMMAND_OPEN;
		command->cmd_valve = parse_valve(&str[strlen(COMMAND_MSG_OPEN) + 1]);
		return 0;
	}

	if (is_prefix(str, COMMAND_MSG_CLOSE)) {
		if (str[strlen(COMMAND_MSG_CLOSE)] != ':') {
			return -1;
		}

		if (strlen(&str[strlen(COMMAND_MSG_CLOSE) + 1]) != 3) {
			return -1;
		}

		command->cmd_type = COMMAND_CLOSE;
		command->cmd_valve = parse_valve(&str[strlen(COMMAND_MSG_CLOSE) + 1]);
		return 0;
	}

	if (is_prefix(str, COMMAND_MSG_IGNITE)) {
		if (strlen(str) == strlen(COMMAND_MSG_IGNITE)) {
			command->cmd_type = COMMAND_IGNITE;
			command->cmd_valve = NONE;
			return 0;
		}
	}

	return -1;
}

line_iterator_t make_line_iterator(char* base) {
	line_iterator_t iter = {
		.base = base,
		.pos = base,
		.current_line = NULL
	};

	return iter;
}

char* line_iterator_next(line_iterator_t* iter) {
	size_t current_line_len = 0;

	// Reached end of base string
	if (*iter->pos == '\0') {
		free(iter->current_line);
		iter->current_line = NULL;
		return NULL;
	}

	// Get past the last iteration's newline
	if (iter->pos[current_line_len] == '\n') {
		iter->pos++;
	}

	// Find the next newline
	while (iter->pos[current_line_len] != '\n' && iter->pos[current_line_len] != '\0') {
		current_line_len++;
	}

	iter->current_line = realloc(iter->current_line, current_line_len + 2);

	if (!iter->current_line) {
		// Out of memory :(
		abort();
	}

	strncpy(iter->current_line, iter->pos, current_line_len + 2);
	iter->current_line[current_line_len + 1] = '\0';
	iter->pos += current_line_len;

	return iter->current_line;
}

text_stream_t make_text_stream(const char* base) {
	char* iter_base = malloc(strlen(base) + 1);

	if (!iter_base) {
		abort();
	}
	
	strcpy(iter_base, base);
	iter_base[strlen(base)] = '\0';

	text_stream_t stream = {
		.line_iter = make_line_iterator(iter_base),
		.remainder = NULL
	};

	return stream;
}

char* text_stream_next(text_stream_t* stream) {
	char* current_line = line_iterator_next(&stream->line_iter);

	if (current_line) {
		stream->remainder = realloc(stream->remainder, strlen(current_line) + 1);
		strcpy(stream->remainder, current_line);
		stream->remainder[strlen(current_line)] = '\0';

		if (current_line[strlen(current_line) - 1] != '\n') {
			return NULL;
		}
	}

	return current_line;
}

void text_stream_buffer(text_stream_t* stream, char* text) {
	if (!stream->remainder) {
		*stream = make_text_stream(text);
		return;
	}

	if (stream->remainder[strlen(stream->remainder) - 1] != '\n') {
		// Reached end of current iterator, so append the remainder.

		stream->line_iter.base = malloc(strlen(stream->remainder) + strlen(text) + 1);

		if (!stream->remainder) {
			abort();
		}
		
		strcat(stream->line_iter.base, stream->remainder);
		strcpy(stream->line_iter.base + strlen(stream->remainder), text);
		stream->line_iter = make_line_iterator(stream->line_iter.base);
		return;
	}
	
	// If we haven't exhausted the iterator, we can append to the end of the
	// iterator's base string
	
	stream->line_iter.base = realloc(
		stream->line_iter.base,
		strlen(stream->line_iter.base) + strlen(text) + 1
	);

	if (!stream->line_iter.base) {
		abort();
	}
	
	strcpy(
		&stream->line_iter.base[strlen(stream->line_iter.base)],
		text
	);
}

void text_stream_destroy(text_stream_t* stream) {
	free(stream->line_iter.base);
	free(stream->line_iter.current_line);
	free(stream->remainder);
}

command_reader_t make_command_reader(char* buf) {
	if (!buf) {
		buf = "";
	}

	return (command_reader_t)make_text_stream(buf);
}

int command_reader_read(command_reader_t* reader, command_t* command) {
	char* line = text_stream_next((text_stream_t*)reader);

	if (!line) {
		return -1;
	}
	
	line[strlen(line) - 1] = '\0';
	return parse_command(line, command);
}

void command_reader_buffer(command_reader_t* reader, char* buf) {
	text_stream_buffer((text_stream_t*)reader, buf);
}
