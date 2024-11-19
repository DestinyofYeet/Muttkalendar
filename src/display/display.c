#include "display.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#define STR_N_A "N/A"
#define TIME_FORMAT_STR "%d.%m.%Y - %H:%M"

char *ics_format_rrule(ICS_RRULE *rrule){
  bool tmp_buffer_allocated = false;
  char *tmp_buffer;

  if (rrule == NULL){
    size_t na_size = sizeof(char) * strlen(STR_N_A);
    char *na = malloc(na_size);
    memcpy(na, STR_N_A, na_size);
    return na;
  }

  if (rrule->interval == 1){
    tmp_buffer = "";
  } else {
    int size_needed = snprintf(NULL, 0, " %d.", rrule->interval);
    tmp_buffer = malloc(sizeof(char) * size_needed);
    tmp_buffer_allocated = true;
    snprintf(tmp_buffer, size_needed, " %d.", rrule->interval);
  }

  char *freq_tmp_buffer;

  if (rrule->frequency == DAILY){
    freq_tmp_buffer = "day";
  } else if (rrule->frequency == WEEKLY){
    freq_tmp_buffer = "week";
  } else if (rrule->frequency == MONTHLY){
    freq_tmp_buffer = "month";
  } else {
    freq_tmp_buffer = "";
  }

  int needed_size = snprintf(NULL, 0, "Occurs every%s %s on %s\n", tmp_buffer, freq_tmp_buffer, rrule->byday) + 1;
  
  char *out_string = malloc(sizeof(char) * needed_size);

  snprintf(out_string, needed_size, "Occurs every%s %s on %s\n",  tmp_buffer, freq_tmp_buffer, rrule->byday);
  if (tmp_buffer_allocated){
    free(tmp_buffer);
  }
  out_string[needed_size] = '\0';
  return out_string;
}

bool ics_exists_timezone(char *timezone){
  char *tz_dir = getenv("TZDIR");

  if (tz_dir == NULL){
    tz_dir = "/usr/share/zoneinfo";
  }
  
  size_t string_size_needed = snprintf(NULL, 0, "%s/%s", tz_dir, timezone) + 1;

  char *check_timezone = malloc(string_size_needed);

  snprintf(check_timezone, string_size_needed, "%s/%s", tz_dir, timezone);

  debug("Checking if file %s exists\n", check_timezone);
  bool exists = access(check_timezone, F_OK) == 0;
  debug("Timezone %s %s\n", check_timezone, exists ? "exists" : "does not exist");
  free(check_timezone);
  // free(tz_dir);
  return exists;
}

char *ics_format_time(ICS_Time *time){
  if (time == NULL){
    char *result = malloc(sizeof(STR_N_A));
    memcpy(result, STR_N_A, sizeof(STR_N_A));
    return result;
  }

  struct tm time_to_convert = time->time;
  bool timezone_exists = ics_exists_timezone(time->timezone);
  
  if (time->timezone != NULL && timezone_exists){
    // if timezone is NULL, we have a floating timezone and we shouldn't do anything to it 
    #ifdef DEBUG
    printf("Setting timezone from %s to %s\n", getenv("TZ"), time->timezone);
    #endif
    
    char *old_tz = getenv("TZ");
    char *tz_buffer = NULL;

    if (old_tz != NULL){
      size_t tz_len = strlen(old_tz);
      tz_buffer = malloc(sizeof(char) * (tz_len + 1));
      strcpy(tz_buffer, old_tz);
    }

    setenv("TZ", time->timezone, 1);
    tzset();

    
    time_t utc_time = mktime(&time->time);
    if (utc_time == -1){
      perror("mktime failed\n");
      return NULL;
    }
  
    if (tz_buffer == NULL){
      unsetenv("TZ");
    } else {
      setenv("TZ", tz_buffer, 1);
    }

    tzset();

    struct tm *local_tm = localtime(&utc_time);
    if (local_tm == NULL){
      perror("localtime failed\n");
      return NULL;
    }

    free(tz_buffer);

    time_to_convert = *local_tm;
  } 

  char *time_format = TIME_FORMAT_STR;

  size_t buffer_size = strlen(time_format) * sizeof(char);

  char *buffer = malloc(sizeof(char) * buffer_size);

  int bytes_written  = strftime(buffer, buffer_size, time_format, &time_to_convert);

  while (bytes_written == 0){
    buffer_size *= 2;
    buffer = realloc(buffer, buffer_size);
    bytes_written  = strftime(buffer, buffer_size, time_format, &time_to_convert);
  }

  if (!timezone_exists){
    char *buffer_format = "%s (%s)";

    debug("Attempting to create following string: %s (%s)\n", buffer, time->timezone);

    size_t buffer_size_needed = snprintf(NULL, 0, buffer_format, buffer, time->timezone) + 1;

    char *new_buffer = malloc(buffer_size_needed);

    snprintf(new_buffer, buffer_size_needed, buffer_format, buffer, time->timezone);
    new_buffer[buffer_size_needed] = '\0';
    free(buffer);
    buffer = new_buffer;
  }

  return buffer;
}

void ics_display_event(void *node, void* extra_args){
  ICS_Event *event = node;
 
  if (event->description != NULL){
    size_t desc_size = strlen(event->description);
    ics_convert_lf_to_real_lf(event->description, desc_size);
  } 

  char *start_time = ics_format_time(event->start);
  char *end_time = ics_format_time(event->end);
  debug("%s\n", "---------------------------------------");
  printf("Event Summary: \t\t%s\n", event->summary);
  //fwrite(description_string, 1, needed_size, stdout);
  // printf("Event Description: \t%s\n", event->description);
  printf("Event Description: \t%s\n", event->description);
  printf("Event Location: \t%s\n", event->location);
  printf("Start time: \t\t%s\n", start_time);
  printf("End time: \t\t%s\n", end_time);
  if (event->rrule != NULL){
    char *rrule_string = ics_format_rrule(event->rrule);
    printf("Reoccuring: \t\t%s\n", rrule_string);
    free(rrule_string);
  }
  printf("\n");
  free(start_time);
  free(end_time);
}

void ics_display_calendar(ICS_File *file){
  list_iterate(file->calendar->events, ics_display_event, NULL);
}
