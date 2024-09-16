#ifndef EVENTS_H
#define EVENTS_H

#include "utils/utils.h"

typedef enum {
  EMPTY,
  DAILY,
  WEEKLY,
  MONTHLY
} ICS_RRULE_Frequency;

typedef struct {
  ICS_RRULE_Frequency frequency;
  int interval;
  // List *byday;
  char *byday;
} ICS_RRULE;

typedef struct {
  char *summary;
  char *uid;
  char *x_smt_category_color;
  List *categories;
  char *description;
  char *location;
  char *last_modified;
  char *transparancy;
  ICS_Time *start;
  ICS_Time *end;
  char *x_smt_missing_year;
  char *timestamp;
  char *status;
  List *alarms;
  ICS_RRULE *rrule;
} ICS_Event;

void ics_parse_multiple_events(void *input, void *data);
ICS_Event *ics_parse_event(char *buffer, size_t buffer_size);
ICS_Event *ics_create_event();

ICS_RRULE *ics_create_rrule();
void ics_destroy_rrule(ICS_RRULE *data);

void ics_destroy_event_void(void *event);
void ics_destroy_event(ICS_Event *event);

#endif
