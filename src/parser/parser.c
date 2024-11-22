#include "parser.h"
#include "modules/event.h"
#include "modules/utils/utils.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <wchar.h>


#define VCALENDER_BEGIN "BEGIN:VCALENDAR"
#define VCALENDER_END "END:VCALENDAR"

#define VEVENT_BEGIN "BEGIN:VEVENT"
#define VEVENT_END "END:VEVENT"

ICS_File* ics_parse_buffer(ICS_Buffer *buffer, ICS_Arguments *args){
  
  if(args->verbose) printf("Read buffer:\n%s\n", buffer->content);


  size_t dest_size = buffer->size * 4; // based on worst case behaviour of 1 -> 4 byte conversion
  char *dest_buffer = malloc(dest_size);

  size_t old_file_size = buffer->size;

  if (args->charset != NULL){
    debug("Converting from %s to UTF-8\n", args->charset);
    ics_convert_encoding(args->charset, buffer->content, buffer->size, dest_buffer, dest_size);
    debug("File size changed from %zu to %zu!\n", old_file_size, dest_size);
    ics_buffer_destroy(buffer);
  } else {
    debug("Skipping conversion!%s", "\n");
    free(dest_buffer);
    dest_buffer = ics_string_copy(buffer->content, buffer->size);
    dest_size = buffer->size;
    ics_buffer_destroy(buffer);
  }


  bool changed = ics_unfold_file(dest_buffer, dest_size, args->verbose);

  if (changed){
    if (args->verbose) printf("Unfolded Buffer:\n%s\n", dest_buffer);
  }

  ics_nuke_backslash_comma(dest_buffer, dest_size);

  ICS_File *ics_file = ics_create_file();

  ICS_Block *vcalender_block = ics_get_block(dest_buffer, dest_size, VCALENDER_BEGIN, sizeof(VCALENDER_BEGIN), VCALENDER_END, sizeof(VCALENDER_END));

  if (vcalender_block == NULL){
    debug("Failed to parse vcalender_block%s\n", "!");
    free(dest_buffer);
    ics_destroy_block(vcalender_block);
    ics_destroy_file(ics_file);
    return NULL;
  } else {
    debug("Vcalender: %s\n", vcalender_block->content);
  }
  
  ICS_Calendar *calendar = NULL; 
  if (vcalender_block != NULL){
    calendar = ics_parse_calendar(vcalender_block->content, vcalender_block->size);
  }

  ics_file->calendar = calendar;

  ics_destroy_block(vcalender_block);

  free(dest_buffer);
  return ics_file;
}

ICS_File *ics_create_file(){
 ICS_File *file = malloc(sizeof(ICS_File));

  file->file_path = NULL;
  file->calendar = NULL;
  return file;
}

void ics_destroy_file(ICS_File *file){
  if (file == NULL) return;

  ics_destroy_calendar(file->calendar);

  free(file);
}

bool ics_unfold_file(char *buffer, size_t buf_size, int8_t verbose){
  // idk why someone invented folding a machine-read document
  // This funtion detects the CRLF + space sequence and deletes it, but everything breaks this shit and converts it to LF
  // CR: 13 LF: 10: Space: 32
  //
  // Not pretty, but it works
  
  char *tmp_buffer = malloc(sizeof(char) * (buf_size + 1));
  int buffer_write_count = 0;
  bool has_replaced = false;

  for (int i=0; i < buf_size; i++){
    int c = buffer[i];
    // checks for the LF + (space || tab) sequence
    if ((c == CHAR_LF) && (i + 1 < buf_size) && ((buffer[i + 1] == CHAR_SPACE) || buffer[i + 1] == CHAR_TAB)){
      // if there is a CR infront of the LF, remove it
      if (buffer[i - 1] == CHAR_CR){
        --buffer_write_count; // decrement the write_pointer so that it overwrites the CR again
      }

      i += 2; // skip the LF and the space

      while (i < buf_size){
        // keeps checking if there are more spaces or tabs, because some programs do that
        if (buffer[i] == CHAR_SPACE || buffer[i] == CHAR_TAB){
          ++i;
        } else {
          debug("Found spaces!%s", "\n");
          has_replaced = true;
          i -= 1; // needed to fix the cutoff of characters
          break;
        }
      }
      continue;
    }

    memcpy(tmp_buffer+buffer_write_count, &c, sizeof(char));
    ++buffer_write_count;
  }

  // maybe re-size the buffer and set that to the buffer var
  memset(buffer, '\0', buf_size);
  memcpy(buffer, tmp_buffer, buffer_write_count);
  free(tmp_buffer);

  return has_replaced; 
}


ICS_Calendar *ics_create_calendar(){
  ICS_Calendar *calendar = malloc(sizeof(ICS_Calendar));

  calendar->prodid = NULL;
  calendar->version = NULL;
  calendar->events = NULL;

  return calendar;
}

void ics_destroy_calendar(ICS_Calendar *calendar){
  if (calendar == NULL) return;

  free(calendar->version);
  free(calendar->prodid);
  list_free(calendar->events, ics_destroy_event_void);
  
  free(calendar);
}

void _ics_parse_calendar(char *line, size_t line_length, void *data){
  ICS_Calendar *calendar = data;

  ICS_KeyValuePair *pair = ics_split_key_value(line, line_length, ':');

  char *value = ics_copy_pair_value(pair);

  if (strcmp(pair->key, "PRODID") == 0){
    calendar->prodid = value;
  } else if (strcmp(pair->key, "VERSION") == 0){
    calendar->version = value;
  } else {
    free(value);
  }

  ics_destroy_pair(pair);
  free(line);
}


ICS_Calendar *ics_parse_calendar(char *buffer, size_t buffer_size){
  ICS_Calendar *calendar = ics_create_calendar();
  
  debug("ics_parse_calendar: About to parse calendar%s\n", "!");
  ics_get_lines(buffer, buffer_size, _ics_parse_calendar, calendar);
  
  List *vevent_blocks = ics_get_all_blocks(buffer, buffer_size, VEVENT_BEGIN, sizeof(VEVENT_BEGIN), VEVENT_END, sizeof(VEVENT_END));

  if (vevent_blocks == NULL || vevent_blocks->size == 0){
    // printf("Failed to parse vevent block!\n");
  } else {
    // printf("Vevent: %s\n", vevent_block->content);
    List *events = list_create();
    list_iterate(vevent_blocks, ics_parse_multiple_events, events);
    
    list_free(vevent_blocks, ics_destroy_block_void);

    calendar->events = events;
  }
  return calendar;
}
