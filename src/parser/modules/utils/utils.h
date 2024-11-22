#ifndef UTILS_H
#define UTILS_H
#define _XOPEN_SOURCE 700
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "list.h"
#include "keys.h"

#ifdef DEBUG
#define _DEBUG 1
#else
#define _DEBUG 0
#endif

#define DEBUG_PREFIX "Debug: "

#define debug(frmt, ...) if (_DEBUG) do { fprintf(stdout, DEBUG_PREFIX frmt, __VA_ARGS__); } while (0)

typedef struct ICS_Block {
  char* content;
  size_t size;
  int64_t buffer_end_position;
} ICS_Block;

typedef struct ICS_Time {
  struct tm time;
  char *timezone;
} ICS_Time;


typedef struct {
  char* key;
  char* value;
  char delimiter;
  size_t key_length;
  size_t value_length;
} ICS_KeyValuePair;

typedef struct {
  int counter;
} ICS_Line_Parse_Data;

typedef struct {
  char *content;
  size_t size;
} ICS_Buffer;


ICS_Block* ics_get_block(const char *buffer, const size_t buffer_size, const char *start_string, const size_t start_size, const char *end_string, const size_t end_size);
ICS_Block* ics_get_block_next(const char* buffer, const size_t buffer_size, const char *start_string, const size_t start_size, const char *end_string, const size_t end_size, const ICS_Block *previous);
ICS_Block *ics_create_block();
void ics_destroy_block(ICS_Block *block);
void ics_destroy_block_void(void *block);

void ics_get_lines(const char *buffer, const size_t buffer_size, void (*func)(char *, size_t, void *), void *arg);

ICS_KeyValuePair *ics_split_key_value(const char *buffer, const size_t buffer_size, const char delimiter);

void ics_destroy_pair(ICS_KeyValuePair *pair);

char *ics_copy_pair_value(ICS_KeyValuePair *pair);

void ics_parse_time(const char *content, struct tm *time_struct, bool has_timezone);

int get_file_line_count(FILE *fp);

ICS_Time *ics_create_time();
void ics_destroy_time(ICS_Time *time);
char *ics_string_copy(char *string, size_t string_length);

void ics_convert_lf_to_real_lf(char *buffer, size_t buffer_size);
void ics_nuke_cr(char *buffer, size_t buffer_size);
void ics_convert_encoding(const char* from_encoding, const char* input, size_t input_len, char* output, size_t output_len);
void ics_nuke_backslash_comma(char *buffer, size_t buffer_size);

ICS_Line_Parse_Data *ics_create_line_parse_data();
void ics_destroy_line_parse_data(ICS_Line_Parse_Data *data);

char *ics_strings_concat(char *first, size_t length_one, char *second, size_t length_two);

ICS_Buffer *ics_buffer_create();
void ics_buffer_destroy(ICS_Buffer *buffer);

ICS_Buffer *ics_read_file(FILE *file);
ICS_Buffer *ics_read_stdin();
#endif 
