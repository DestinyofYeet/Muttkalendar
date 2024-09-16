#include "utils.h" 
#include <iconv.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>

int get_file_line_count(FILE *fp){
  char ch = 1;
  int lines = 0;
    while(ch != EOF) {
      ch = fgetc(fp);
      if(ch == '\n') {
        lines++;
      }
    }
  
  return lines;
}

void ics_parse_time(const char *content, struct tm *time_struct, bool has_timezone){
  #ifdef DEBUG
  printf("ics_parse_time: content: %s\n", content);
  #endif 
  
  memset(time_struct, '\0', sizeof(struct tm));
  
  strptime(content, "%Y%m%dT%H%M%S", time_struct);
  
}

void ics_get_lines(const char *buffer, const size_t buffer_size, void (*func)(char *,  size_t, void *), void *arg){
  // this should find out how long a line is, create a string that is exactly that size
  // and give it to a function that decides how to parse that string
  size_t line_start_pointer = 0;

  ICS_Line_Parse_Data *line_parse_data = ics_create_line_parse_data();

  for (int64_t i=0; i < buffer_size; i++){
    int c = buffer[i];
    // checks for CRLF sequence aka end of line. Is what I would like to say, but everything breaks that and converts it to LF
    if (c == CHAR_LF){

      size_t line_length = i - line_start_pointer;

      // I just want the line and not the CRLF if it exists
      if (buffer[i - 1] == CHAR_CR){
        --line_length;
      }

      char *complete_line = malloc(sizeof(char) * (line_length + 1));
      
      memcpy(complete_line, buffer+line_start_pointer, i - line_start_pointer);
      complete_line[line_length] = '\0';

      debug("ics_get_lines: Complete line: %s\n", complete_line);

      ICS_KeyValuePair *pair = ics_split_key_value(complete_line, line_length, ':');
      
      debug("ics_get_lines: Key: %s | Counter: %d!\n", pair->key, line_parse_data->counter);

      if (strcmp(pair->key, "BEGIN") == 0){
        debug("ics_get_lines: Increasing counter from %d to %d\n", line_parse_data->counter, line_parse_data->counter + 1);
        line_parse_data->counter += 1;
      } else if (strcmp(pair->key, "END") == 0){
        debug("ics_get_lines: Decreasing counter from %d to %d\n", line_parse_data->counter, line_parse_data->counter - 1);
        line_parse_data->counter -= 1;
      }

      if (line_parse_data->counter != 0){
        debug("ics_get_lines: Skipping current line: %s\n", complete_line);
        free(complete_line);
        ics_destroy_pair(pair);
        line_start_pointer = i + 1; // +1 because we want to point to the start of the next line
        continue; 
      } else {
        func(complete_line, line_length, arg);
      }
 
      ics_destroy_pair(pair);
      line_start_pointer = i + 1; // +1 because we want to point to the start of the next line
    }
  }

  ics_destroy_line_parse_data(line_parse_data);
}

ICS_KeyValuePair *ics_split_key_value(const char *buffer, const size_t buffer_size, const char delimiter){
  // this function will split a KEY:VALUE pair and allocate a string for it
  int64_t split_index = -1;

  for (int64_t i = 0; i < buffer_size; i++){
    char c = buffer[i];

    if (c == delimiter){
      split_index = i;
      break;
    }
  }

  if (split_index == -1) return NULL;

  ICS_KeyValuePair *pair = malloc(sizeof(ICS_KeyValuePair));

  pair->delimiter = delimiter;

  pair->key = malloc(sizeof(char) * (split_index + 1));
  pair->value = malloc(sizeof(char) * (buffer_size - split_index));
  
  memcpy(pair->key, buffer, split_index);
  pair->key[split_index] = '\0';
  pair->key_length = split_index;

  memcpy(pair->value, buffer+split_index + 1, buffer_size - split_index - 1);
  pair->value[buffer_size - split_index - 1] = '\0';
  pair->value_length = buffer_size - split_index - 1;

  return pair;
}

char *ics_copy_pair_value(ICS_KeyValuePair *pair){
  char *buffer = malloc(sizeof(char) * (pair->value_length + 1));
  memcpy(buffer, pair->value, pair->value_length);
  buffer[pair->value_length] = '\0';
  return buffer;
}

char *string_copy(char *string, size_t string_length){
  // thought I needed it, but I don't
  // The only use-case could just use ics_copy_pair_value
  char *new_string = malloc(sizeof(char) * (string_length + 1));

  memset(new_string, '\0', string_length);

  memcpy(new_string, string, string_length);
  new_string[string_length] = '\0';

  return new_string;
}

void ics_destroy_pair(ICS_KeyValuePair *pair){
  if (pair == NULL) return;

  free(pair->key);
  free(pair->value);
  free(pair);
}

List *ics_get_all_blocks(const char *buffer, const size_t buffer_size, const char *start_string, const size_t start_size, const char *end_string, const size_t end_size){
  // Get multiple blocks like VEVENT and puts them in a linked list
  ICS_Block *start = ics_get_block(buffer, buffer_size, start_string, start_size, end_string, end_size);

  if (start == NULL) return NULL;
  
  List *alarms = list_create();
  
  while (start != NULL){
    list_append(alarms, start);

    start = ics_get_block_next(buffer, buffer_size, start_string, start_size, end_string, end_size, start);
  }

  return alarms;
}


ICS_Block* ics_get_block(const char *buffer, const size_t buffer_size, const char *start_string, const size_t start_size, const char *end_string, const size_t end_size){
  // This function is supposed to get blocks, like VCALENDER, VEVENT or VALARM
  char* start = strstr(buffer, start_string);
  if (start == NULL) return NULL;
  int position_start = start - buffer;

  char* end = strstr(buffer, end_string);
  if (end == NULL) {
    debug("WARNING: Could not find end string: %s\n", end_string);

    return NULL;
  }
  int position_end = end - buffer;

  if (position_start > position_end){
    debug("ERROR: ics_get_block: start is bigger than end!%s", "\n");
    return NULL;
  }

  int block_size = end - start + end_size;

  // printf("Block size: %d | Start pos: %d | End pos: %d\n", block_size, position_start, position_end);
  char* block = malloc(sizeof(char) * (block_size + 1));
  memcpy(block, buffer+position_start, block_size);

  ICS_Block *ics_block = ics_create_block();

  block[block_size] = '\0';
 
  ics_block->content = block;
  ics_block->size = block_size;
  ics_block->buffer_end_position = position_end + end_size;

  return ics_block;
}

ICS_Block* ics_get_block_next(const char* buffer, const size_t buffer_size, const char *start_string, const size_t start_size, const char *end_string, const size_t end_size, const ICS_Block *previous){
  size_t position = previous->buffer_end_position;
  if (position > buffer_size){
    return NULL;
  }
  ICS_Block *block = ics_get_block(buffer+position,buffer_size-position, start_string, start_size, end_string, end_size);

  if (block == NULL) return NULL;

  if (strcmp(previous->content, block->content) == 0 && previous->size == block->size){
    ics_destroy_block(block);
    return NULL;
  }

  return block;
}


ICS_Block *ics_create_block(){
  ICS_Block *block = malloc(sizeof(ICS_Block));
  block->content = NULL;
  block->size = 0;
  block->buffer_end_position = -1;
  return block;
}

void ics_destroy_block_void(void *block){
  ics_destroy_block(block);
}

void ics_destroy_block(ICS_Block *block){
  if (block == NULL) return;
  free(block->content);
  free(block);
}

ICS_Time *ics_create_time(){
  ICS_Time *time = malloc(sizeof(ICS_Time));

  struct tm time_struct;
  
  memset(&time_struct, '\0', sizeof(struct tm));

  time->time = time_struct;
  time->timezone = NULL;

  return time;
}

void ics_destroy_time(ICS_Time *time){
  if (time == NULL) return;
  free(time->timezone);
  
  free(time);
}

void ics_convert_lf_to_real_lf(char *buffer, size_t buffer_size){
  size_t buffer_counter = 0;

  for (size_t i = 0; i <= buffer_size; i++){
    char c = buffer[i];

    if (c == CHAR_BACKSLASH && (( i + 1 <= buffer_size) && buffer[i + 1] == CHAR_N)){
      debug("Found fake newline: %c | %c | (%zu)\n", c, buffer[i + 1], i);
      i += 1; // only +1, since the for loop also adds one and we just want to skip two chars
      buffer[buffer_counter] = CHAR_LF;
    } else {
      buffer[buffer_counter] = c;
    }

    buffer_counter++;
  }
}

void ics_nuke_cr(char *buffer, size_t buffer_size){
  // was built for testing purposes, not used anymore
  size_t buffer_counter = 0;

  for (size_t i = 0; i <= buffer_size; i++){
    char c = buffer[i];

    if (c == CHAR_CR){
      debug("Found CR: %c | %c | (%zu)\n", c, buffer[i + 1], i);
      i += 1; // only +1, since the for loop also adds one and we just want to skip two chars
      buffer[buffer_counter] = CHAR_LF;
    } else {
      buffer[buffer_counter] = c;
    }

    buffer_counter++;
  }
}

void ics_nuke_backslash_comma(char *buffer, size_t buffer_size){
  size_t buffer_counter = 0;

  for (size_t i = 0; i <= buffer_size; i++){
    char c = buffer[i];

    if (c == CHAR_BACKSLASH && (i + 1 < buffer_size && buffer[i + 1] == CHAR_COMMA)){
      debug("Found backslash before comma%s", "\n");
      // do nothing, since the the loop will skip over it
    } else {
      buffer[buffer_counter] = c;
      buffer_counter++;
    }
  }
}

void ics_convert_encoding(const char* from_encoding, const char* input, size_t input_len, char* output, size_t output_len) {
  // ISO-8859-1 = latin-1
  iconv_t cd = iconv_open("UTF-8", from_encoding);
  if (cd == (iconv_t)-1) {
      perror("iconv_open");
      exit(EXIT_FAILURE);
  }

  char *in_buf = (char *)input;
  char *out_buf = output;
  size_t in_bytes_left = input_len;
  size_t out_bytes_left = output_len;

  size_t result = iconv(cd, &in_buf, &in_bytes_left, &out_buf, &out_bytes_left);
  if (result == (size_t)-1) {
      perror("iconv");
      iconv_close(cd);
      exit(EXIT_FAILURE);
  }

  iconv_close(cd);
}

ICS_Line_Parse_Data *ics_create_line_parse_data(){
  ICS_Line_Parse_Data *data = malloc(sizeof(ICS_Line_Parse_Data));
  data->counter = -1;
  return data;
}
void ics_destroy_line_parse_data(ICS_Line_Parse_Data *data){
  free(data);
}
