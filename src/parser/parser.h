#ifndef PARSER_H_
#define PARSER_H_

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "modules/utils/list.h"
#include "modules/utils/utils.h"
#include "modules/event.h"

typedef struct {
  char *prodid;
  char *version;
  List *events;
} ICS_Calendar;

typedef struct {
  char *buffer;
  size_t size;
  size_t current_write_pos;
} ICS_Convert_Struct;


typedef struct ICS_File {
  char* file_path;
  ICS_Calendar *calendar;
} ICS_File;

typedef struct {
//  char *args[1];
  bool verbose;
  char *input_file;
  char *charset;
} ICS_Arguments;

ICS_Calendar *ics_parse_calendar(char *buffer, size_t buffer_size);

ICS_Calendar *ics_create_calendar();
void ics_destroy_calendar(ICS_Calendar *calendar);

ICS_File *ics_parse_file(FILE *file, ICS_Arguments *args);
bool ics_unfold_file(char* buffer, size_t size, int8_t debug);

ICS_File *ics_create_file();
void ics_destroy_file(ICS_File *file);
void ics_parse_line(char* line, size_t length, ICS_File *event);

char *ics_convert_LF_CRLF(char *buffer, size_t buf_size, size_t *new_buf_size, int line_count);
#endif // PARSER_H_
