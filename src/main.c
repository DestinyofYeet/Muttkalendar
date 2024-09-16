#include "parser/parser.h"
#include <stdio.h>
#include <argp.h>
#include <stdlib.h>
#include <string.h>
#include "display/display.h"

const char *argp_program_version =
  "0.1";

const char *argp_program_bug_address =
  "ole@uwuwhatsthis.de";

static char doc[] =
  "A ics parser for Mutt";

// static char args_doc[] = "Arg1";
static char args_doc[] = "";

static struct argp_option options[] = {
  {"verbose",   'v', 0,   0, "Produce verbose output" },
  {"input",    'i', "FILE", 0, "Read input from FILE"}, 
  {"charset", 'c', "CHARSET", 0, "Convert from the specified charset to UTF-8"},
  {0}
};


static error_t parse_opt(int key, char *arg, struct argp_state *state){
  ICS_Arguments *arguments = state->input;

  switch (key){
    case 'v':
      arguments->verbose = true;
      break;
    case 'i':
      arguments->input_file = arg;
      break;
    case 'c':
      arguments->charset = arg;
      break;
/*    case ARGP_KEY_ARG:
      if (state->arg_num >= 1) argp_usage(state);
      
      arguments->args[state->arg_num] = arg;
      break;

    case ARGP_KEY_END:
      if (state->arg_num < 1){
        argp_usage(state);
      }

      break;
*/
    default:
      return ARGP_ERR_UNKNOWN;
  }

  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

int main(int argc, char **argv){

  ICS_Arguments arguments;

  arguments.input_file = "-";
  arguments.verbose = 0;
  arguments.charset = NULL;

  argp_parse(&argp, argc, argv, 0, 0, &arguments);

  if (arguments.verbose){
      printf("Input_file = %s\nVerbose = %s\n",
 //        arguments.args[0],
         arguments.input_file,
         arguments.verbose ? "yes" : "no");
  }

  if (strcmp(arguments.input_file, "-") == 0){
    printf("Reading from stdin is not supported yet.\n");
    return 1;
  }

  FILE *fptr;

  if ((fptr = fopen(arguments.input_file, "r")) == NULL){
    printf("Error! Could not open the file!\n");
    return 1;
  }

  ICS_File *ics_file = ics_parse_file(fptr, &arguments);

  fclose(fptr);

  if (ics_file == NULL){
    printf("Failed to parse file: %s!\n", arguments.input_file);
    return 1;
  }

  ics_file->file_path = arguments.input_file;

  ics_display_calendar(ics_file);

  ics_destroy_file(ics_file);

  return 0;
}
