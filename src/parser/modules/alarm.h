#ifndef ALARMS_H
#define ALARMS_H
#include <stdlib.h>
#include "utils/utils.h"

typedef struct {
  char *description;
  char *action;
  char *trigger;
} ICS_Alarm;


ICS_Alarm *ics_create_alarm();
void ics_destroy_alarm(ICS_Alarm *alarm);
void ics_destroy_alarm_void(void *alarm);
ICS_Alarm *ics_parse_alarm(char *buffer, size_t buffer_size);
void ics_parse_multiple_alarms(void *input, void *data);
#endif 
