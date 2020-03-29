#ifndef OPTIONS_H
#define OPTIONS_H

/**
 * options.h
 */

enum option_type {
        OPTION_TYPE_NO_OPTION = 0,
        OPTION_TYPE_NUMERIC = 1, // INT
        OPTION_TYPE_STRING = 2
};

/* option_match() return codes. */

#define OPTION_FOUND             0  
#define OPTION_UNKNOWN_OPTION   -1 // option not set call to options_parse()
#define OPTION_INVALID_ARGUMENT -2 // next arg is not int or string
#define OPTION_MISSING_ARGUMENT -3 // Run out of cmd line args.

struct option_str {
        char *long_option;
        char *short_option;
        
        enum option_type type;
        /* If is_set = 1 then the option was found in the command line
        * and the argument set, or, if the option takes no argument, the
        * option was in the command line. If the command takes an argument
        * (type == NUMBER || STRING) then we look at the value which is 
        * guaranteed to be set; string is nul-teminated; strlen(string) can
        * be 0, though, if '' is given).
        *
        * And is is_set = 0 kilo will skip it.
        */
        int is_set;
        
        union {
                int numeric;
                char *string;
        } value;
        
        struct option_str *next; 
};

typedef struct option_str Option;

/**
 * Returns a pointer to a list of option(s) or NULL (no options).
 *  The list is linked.
 *  
 * Updates &next to be the next unprocessed command line argument
 * (eg file name). Note: -- is a recognized opion - end of options
 * which is gobbled.
 *
 * Supports long ("--file") and short ("-f") options. Options can 
 * have 0 or 1 parameters: integer or string. 
 * 
 * The options call parameter has the following format:
 * "O(,O)*" where O = long|s(:i|s)?
 *
 * "file|f:s" => --file options.c | -f options.c 
 * "count|c:i => --count 67 | -c 67
 * "help|h"   => --help | -h
 * 
 * An empty / NULL options string is valid. It matches to 0 or 1 "--".
 */
Option *
options_parse(int argc, char **argv, char *options, int *next);

/**
 *
 */
int
options_find(int argc, char **argv, Option *list, int *index); 

#endif
