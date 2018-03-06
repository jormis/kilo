#ifndef HIGHLIGHT_H
#define HIGHLIGHT_H

/*
`erow.hl' is an array of unsigned char values, meaning integers in the range of 0 to 255. 
Each value in the array will correspond to a character in render, and will tell you whether that 
character is part of a string, or a comment, or a number, and so on. Let’s create an enum containing 
the possible values that the hl array can contain.
*/
enum editor_highlight {
	HL_NORMAL = 0,
	HL_COMMENT, 
	HL_MLCOMMENT,
	HL_KEYWORD1,
	HL_KEYWORD2,
	HL_STRING,
	HL_NUMBER,
	HL_MATCH
};

/* */
#define HL_HIGHLIGHT_NUMBERS (1<<0)
#define HL_HIGHLIGHT_STRINGS (1<<1)

#endif

