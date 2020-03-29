#include "options.h"
#include "help.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/*
 * options.h 
 */

/**
 *
 * Side effects: 1/ *index points to the next argument to be preocessed 
 *  after the function call; The contents of the list may be updated.
 * 
 * return 0 if there was a match
 */
int
options_find(int argc, char **argv, Option *list, int *index) {
        char *p = argv[*index]; 
        while (*p && *p == '-')
                p++;
        
        Option *opt = list;
        do {                
                if (strcmp(opt->short_option, p) == 0
                        || strcmp(opt->long_option, p) == 0) {
                        
                        if (opt->type == OPTION_TYPE_NO_OPTION) {
                                opt->is_set = 1; 
                                return OPTION_FOUND;                                 
                        }
                        
                        if (opt->type == OPTION_TYPE_STRING
                                || opt->type == OPTION_TYPE_NUMERIC) {
                                if (*index < argc - 1) {
                                        *index = *index + 1;
                                        if (opt->type == OPTION_TYPE_STRING) {
                                                opt->value.string = strdup(argv[*index]);
                                        } else {
                                                // TODO ERROR HANDLING
                                                opt->value.numeric = atoi(argv[*index]);       
                                        }
                                        opt->is_set = 1; 
                                        return OPTION_FOUND;        
                                } 
               
                                return OPTION_MISSING_ARGUMENT;
                        }
                        
                }
                opt = opt->next;
        } while (opt != NULL);
        
        //
        return OPTION_UNKNOWN_OPTION;
}

Option *
options_parse(int argc, char **argv, char *options, int *next) {
        /* There are no command line arguments to process. */
        if (argc == 1) {
                *next = 1;
                return NULL;        
        }
               
        if (options == NULL || strlen(options) == 0) {
                if (argc > 1) {
                        /* Gobble '--'. */
                        if (! strcmp(argv[1], "--")) {
                                *next = 2; 
                                return NULL; 
                        }
                } 

                /* argc == 1, nothing to process.  */                
                *next = 1;
                return NULL;
        }

        // char *options is a comma-separated list of options.

        Option *list = NULL; 
        
        char *options_copy = strdup(options);
        char *token = strtok(options_copy, ",");
                
        while (token != NULL) {
                char *long_option = NULL;
                char *short_option = NULL;
                enum option_type type = OPTION_TYPE_NO_OPTION; 
        
                // parse 'long|short[:i|s]'
                
                char *p = token; 
                while (p && *p && *p != '|')
                        p++;
                        
                // Can long_option length == 1?                 
                long_option = strndup(token, p-token);
                
                
                if (!*p) {
                        *next = -1; // TODO BAD ERROR MECH.
                        free(options_copy);
                        return NULL; 
                }
                        
                p++;
                char *start_of_s = p; 
                
                while (*p && *p != ':') 
                        p++;
                        
                short_option = strndup(start_of_s, p-start_of_s);
                
                if (*p == ':' && *(p+1)) {
                        p++;
                        if (*p == 'i') {
                                type = OPTION_TYPE_NUMERIC;
                        } else if (*p == 's') {
                                type = OPTION_TYPE_STRING;
                        } else {
                                *next = -1; // TODO BAD ERROR MECH.
                                free(options_copy);
                                return NULL;         
                        }
                }
        
                Option *opt             = malloc(sizeof (Option)); 
                opt->long_option        = long_option;
                opt->short_option       = short_option;
                opt->type               = type;
                opt->is_set             = 0;     
                opt->next               = NULL; 

                if (list == NULL) {
                        list = opt;
                } else {
                        opt->next = list->next; 
                        list->next = opt;
                }
                                        
                token = strtok(NULL, ",");
        }

        free(options_copy);
                
        // Finally, parse arguments.
        // Store the argument to the same list: Option.value. 
        // Remove unused options, allow only one value per option.
        // Barf and die is there is an unknown option. Or call for help.        
        int parsed = 0; 
        int i = 1; 
        
        do {
                /* No dealing with '-'. */
                if (! strcmp(argv[i], "-")) {
                        printf("Unknown option - (STDIN read not yet implemented).\r\n\r\n");
                        display_help();
                        exit(1);       
                }
                
                /* '--' ends option parsing. */
                if (! strcmp(argv[i], "--")) {
                        // No more options. Advance next. 
                        parsed = 1; 
                        i++;
                } else {
                        // TODO How to deduce that the argument is actually a file? 
                        // We have to backtrack, ie --i so that the file gets opened.
                        // If -- is optional, then files starting with - or -- can't 
                        // be recognized unless prepended by './'.
                                                
                        if (strlen(argv[i]) >= 1 && argv[i][0] == '-') {
                                /* We don't deal with a single '-', so no STDIN read yet 
                                   (when forth is added then we can manipulate text like sed?)
                                   
                                   i = last processed index
                                */
                                int rc = options_find(argc, argv, list, &i);
                                
                                if (rc == OPTION_UNKNOWN_OPTION) {
                                        printf("Unknown option: %s.\r\n\r\n", 
                                                argv[i]);
                                        display_help();
                                        exit(1);        
                                } else if (rc == OPTION_INVALID_ARGUMENT) {
                                        printf("Invalid argument to option %s: '%s'\r\n\r\n", 
                                                argv[i-1], argv[i]);
                                        display_help();
                                        exit(1);        
                                } else if (rc == OPTION_MISSING_ARGUMENT) {
                                        printf("Missing argument for option %s\r\n\r\n", 
                                                argv[i]);
                                        display_help();
                                        exit(1);
                                }
                                
                                i++;
                                
                                // Ok. go on.
                        } else {
                                // The argument does not start with -, so end parsing.
                                parsed = 1; 
                        }
                }
        } while (i < argc && ! parsed);

        /* int *next points to the next index in argv after the command 
         * options have been processed. Note: if *next == argc then there
         * are not files (=command line arguments) to process. 
         * 
         * In other words:
         * The i++ expression at the end of the do-while loop makes sure
         * that next is the next unparsed command line argument (or == argc,
         * in which case nothing to process.)
         */
        *next = i; 
        
        //options_remove_unused(list); 
                               
        return list;  
}
