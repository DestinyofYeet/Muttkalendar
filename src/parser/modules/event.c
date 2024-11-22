#include "event.h"
#include "alarm.h"
#include "utils/utils.h"
#include <malloc.h>
#include <string.h>
#include <stdbool.h>

#define SUMMARY "SUMMARY"
#define DESCRIPTION "DESCRIPTION"
#define LOCATION "LOCATION"
#define UID "UID"
#define CATEGORIES "CATEGORIES"
#define LAST_MODIFIED "LAST-MODIFIED"

#define DTSTART "DTSTART"
#define DTEND "DTEND"

#define RRULE "RRULE"

#define VALARM_BEGIN "BEGIN:VALARM"
#define VALARM_END "END:VALARM"

#define UTC_TIMEZONE "UTC"

#define comp(VALUE, COMP_VALUE) (strcmp(VALUE, COMP_VALUE) == 0)

ICS_Time *_ics_parse_time_from_line(ICS_KeyValuePair *pair){

  ICS_Time *time = ics_create_time();

  if (pair->delimiter == ':'){
    // Timezone is not specified / May be UTC (if prepended with a 'Z')
    if (strcmp(pair->value+(pair->value_length - 1), "Z") == 0){
      // Is UTC Time
      time->timezone = malloc(sizeof(UTC_TIMEZONE) + 1);
      memcpy(time->timezone, "UTC\0", sizeof(UTC_TIMEZONE) + 1);
    }

    ics_parse_time(pair->value, &time->time, false);
  } else if (pair->delimiter == ';'){
    // Timezone is specified
    ICS_KeyValuePair *timezone_data = ics_split_key_value(pair->value, pair->value_length, ':');

    #ifdef DEBUG
    printf("Timezone: %s | Value: %s\n", timezone_data->key, timezone_data->value);
    #endif 

    ics_parse_time(timezone_data->value, &time->time, true);

    ICS_KeyValuePair *timezone_only_data = ics_split_key_value(timezone_data->key, timezone_data->key_length, '=');
    #ifdef DEBUG
    printf("Timezone only key: %s | Value: %s\n", timezone_only_data->key, timezone_only_data->value);
    #endif
    time->timezone = malloc(sizeof(char) * (timezone_only_data->value_length + 1));
    memcpy(time->timezone, timezone_only_data->value, timezone_only_data->value_length);
    time->timezone[timezone_only_data->value_length] = '\0';

    ics_destroy_pair(timezone_data);
    ics_destroy_pair(timezone_only_data);
  }

  return time;
}

ICS_RRULE *_ics_parse_rrule(char *value, size_t value_length){
  ICS_RRULE *rrule = ics_create_rrule();
  
  debug("rrule: %s\n", value);

  ICS_KeyValuePair *freq_pair = ics_split_key_value(value, value_length, ';');

  ICS_KeyValuePair *frequency_pair = ics_split_key_value(freq_pair->key, freq_pair->key_length, '=');
  
  if (comp(frequency_pair->value, "DAILY")){
    rrule->frequency = DAILY;
  } else if (comp(frequency_pair->value, "WEEKLY")){
    rrule->frequency = WEEKLY;
  } else if (comp(frequency_pair->value, "MONTHLY")){
    rrule->frequency = MONTHLY;
  }

  ICS_KeyValuePair *interval_pair = ics_split_key_value(freq_pair->value, freq_pair->value_length, ';');

  ICS_KeyValuePair *real_interval_pair = ics_split_key_value(interval_pair->key, interval_pair->key_length, '=');

  rrule->interval = atoi(real_interval_pair->value);

  ICS_KeyValuePair *byday_pair = ics_split_key_value(interval_pair->value, interval_pair->value_length, '=');

  rrule->byday = ics_string_copy(byday_pair->value, byday_pair->value_length);

  ics_destroy_pair(interval_pair);
  ics_destroy_pair(freq_pair);
  ics_destroy_pair(frequency_pair);
  ics_destroy_pair(byday_pair);
  ics_destroy_pair(real_interval_pair);
  return rrule;
}

ICS_KeyValuePair *_ics_split_key_with_specified_delim(char *line, size_t line_length, char delimiter, char *key, bool *changed){
  ICS_KeyValuePair *pair = ics_split_key_value(line, line_length, delimiter);

  if (strcmp(pair->key, key) == 0){
    *changed = true;
    return pair;
  }

  ics_destroy_pair(pair);
  return NULL;
}

void _ics_parse_event_line(char *line, size_t line_length, void* data){
  ICS_Event *event = data;

  // Could be optimized
  ICS_KeyValuePair *pair = ics_split_key_value(line, line_length, ';');

  if (pair == NULL){
    pair = ics_split_key_value(line, line_length, ':');
  }

  if (pair == NULL){
    debug("WARNING: _ics_parse_event_line: pair is NULL%s\n", "!");
    return;
  }

  if (strcmp(pair->key, SUMMARY) == 0){
    ICS_KeyValuePair *language_split = ics_split_key_value(pair->value, pair->value_length, ':');
    if (language_split != NULL){ // language has been found
      event->summary = ics_copy_pair_value(language_split);
      ics_destroy_pair(language_split);
    } else {
      event->summary = ics_copy_pair_value(pair);
    }

  } else if comp(pair->key, UID){
    event->uid = ics_copy_pair_value(pair);

  } else if comp(pair->key, DESCRIPTION){
    ICS_KeyValuePair *language_split = ics_split_key_value(pair->value, pair->value_length, ':');
    if (language_split != NULL){ // language has been found
      event->description= ics_copy_pair_value(language_split);
      ics_destroy_pair(language_split);
    } else {
      event->description = ics_copy_pair_value(pair);
    }

  } else if comp(pair->key, LOCATION){
    ICS_KeyValuePair *language_split = ics_split_key_value(pair->value, pair->value_length, ':');
    if (language_split != NULL){ // language has been found
      event->location = ics_copy_pair_value(language_split);
      ics_destroy_pair(language_split);
    } else {
      event->location = ics_copy_pair_value(pair);
    }

  } else if (comp(pair->key, DTSTART)){ 
    event->start = _ics_parse_time_from_line(pair); 

  } else if comp(pair->key, DTEND){
    event->end = _ics_parse_time_from_line(pair);

  } else if(comp(pair->key, RRULE)){
    event->rrule = _ics_parse_rrule(pair->value, pair->value_length); 
  }

  ics_destroy_pair(pair);

  bool changed = false;
  pair = _ics_split_key_with_specified_delim(line, line_length, ':', RRULE, &changed);

  if (changed){
    event->rrule = _ics_parse_rrule(pair->value, pair->value_length);
    ics_destroy_pair(pair);
  }

  free(line);
}


void ics_parse_multiple_events(void *input, void *data){ 
  ICS_Block *block = input;
  List *data_list = data;

  ICS_Event *event = ics_parse_event(block->content, block->size); 
  list_append(data_list, event);
}

ICS_Event *ics_parse_event(char *buffer, size_t buffer_length){
  ICS_Event *event = ics_create_event();
  debug("ics_parse_event: About to parse event:\n%s\n", buffer);
  ics_get_lines(buffer, buffer_length, _ics_parse_event_line, event);

  List *alarm_blocks = ics_get_all_blocks(buffer, buffer_length, VALARM_BEGIN, sizeof(VALARM_BEGIN), VALARM_END, sizeof(VALARM_END));

  List *real_alarms = list_create();

  if (alarm_blocks == NULL){
    #ifdef DEBUG
    printf("Failed to get all alarms!\n");
    #endif
  } else {
    list_iterate(alarm_blocks, ics_parse_multiple_alarms, real_alarms);
  }

  #ifdef DEBUG
  printf("We have %ld alarms\n", real_alarms->size);
  #endif

  event->alarms = real_alarms;

  list_free(alarm_blocks, ics_destroy_block_void);
  return event;
}

ICS_RRULE *ics_create_rrule(){
  ICS_RRULE *rrule = malloc(sizeof(ICS_RRULE));

  rrule->byday = NULL;
  rrule->interval = 0;
  rrule->frequency = EMPTY;

  return rrule;
}

void _ics_destroy_rrule_void(void *data){
  // data should be a char
  free(data);
}

void ics_destroy_rrule(ICS_RRULE *rrule){
  if (rrule == NULL) return;
  
  if (rrule->byday != NULL){
    // list_free(rrule->byday, _ics_destroy_rrule_void);
    free(rrule->byday);
  }

  free(rrule);
}

ICS_Event *ics_create_event(){
  ICS_Event *event = malloc(sizeof(ICS_Event));

  event->end = NULL;
  event->uid = NULL;
  event->start = NULL;
  event->status = NULL;
  event->summary = NULL;
  event->timestamp = NULL;
  event->categories = NULL;
  event->transparancy = NULL;
  event->last_modified = NULL;
  event->x_smt_missing_year = NULL;
  event->x_smt_category_color = NULL;
  event->alarms = NULL;
  event->description = NULL;
  event->location = NULL;
  event->rrule = NULL;

  return event;
}

void ics_destroy_event_void(void *event){
  ics_destroy_event(event);
}

void ics_destroy_event(ICS_Event *event){
  if (event == NULL) return;

  // free(event->end);
  free(event->uid);
  // free(event->start);
  free(event->status);
  free(event->summary);
  free(event->timestamp);
  free(event->categories);
  free(event->transparancy);
  free(event->last_modified);
  free(event->x_smt_missing_year);
  free(event->x_smt_category_color);
  list_free(event->alarms, ics_destroy_alarm_void);
  free(event->description);
  free(event->location);
  ics_destroy_time(event->start);
  ics_destroy_time(event->end);
  ics_destroy_rrule(event->rrule);

  free(event);
}
