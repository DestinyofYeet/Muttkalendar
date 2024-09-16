#include "alarm.h"
#include <stdio.h>
#include <malloc.h>
#include "utils/utils.h"
#include <string.h>

#define DESCRIPTION "DESCRIPTION"
#define ACTION "ACTION"
#define TRIGGER "TRIGGER"

void _ics_parse_alarm(char *line, size_t length, void *arg){
  ICS_Alarm *alarm = arg;

  ICS_KeyValuePair *pair = ics_split_key_value(line, length, ':');
  debug("Alarm: %p | line: %s | key: %s | pair: %s\n", alarm, line, pair->key, pair->value);

  char* key = pair->key;
  char* value = ics_copy_pair_value(pair);

  if (strcmp(DESCRIPTION, key) == 0){
    alarm->description = value;
  } else if (strcmp(ACTION, key) == 0){
    alarm->action = value;
  } else if (strcmp(TRIGGER, key) == 0){
    alarm->trigger = value;
  } else {
    free(value);
  }

  ics_destroy_pair(pair);
  free(line);
}

ICS_Alarm *ics_parse_alarm(char *buffer, size_t buffer_size){
  ICS_Alarm *alarm = ics_create_alarm();
  debug("ics_parse_alarm: About to parse an alarm!\n%s\n", buffer);
  ics_get_lines(buffer, buffer_size, _ics_parse_alarm, alarm);
  debug("Action is: %s\n", alarm->action);
  return alarm;
}

void ics_parse_multiple_alarms(void *input, void *data){
  ICS_Block *block = input;
  List *data_list = data;

  ICS_Alarm *alarm = ics_parse_alarm(block->content, block->size); 
  list_append(data_list, alarm);
}


ICS_Alarm *ics_create_alarm(){
  ICS_Alarm *alarm = malloc(sizeof(ICS_Alarm));
  alarm->action = NULL;
  alarm->trigger = NULL;
  alarm->description = NULL;

  return alarm;
}

void ics_destroy_alarm_void(void *alarm){
  ics_destroy_alarm(alarm);
}

void ics_destroy_alarm(ICS_Alarm *alarm){
  if (alarm == NULL) return;

  free(alarm->description);
  free(alarm->trigger);
  free(alarm->action);

  free(alarm);
}
