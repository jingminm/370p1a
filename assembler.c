/**
 * Project 1
 * Assembler code fragment for LC-2K
 */

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

//Every LC2K file will contain less than 1000 lines of assembly.
#define MAXLINELENGTH 1000

int readAndParse(FILE *, char *, char *, char *, char *, char *);
static void checkForBlankLinesInCode(FILE *inFilePtr);
static inline int isNumber(char *);
static inline void printHexToFile(FILE *, int);

int 
main(int argc, char **argv)
{
    char *inFileString, *outFileString;
    FILE *inFilePtr, *outFilePtr;
    char label[MAXLINELENGTH], opcode[MAXLINELENGTH], arg0[MAXLINELENGTH],
            arg1[MAXLINELENGTH], arg2[MAXLINELENGTH];

    if (argc != 3) {
        printf("error: usage: %s <assembly-code-file> <machine-code-file>\n",
            argv[0]);
        exit(1);
    }

    inFileString = argv[1];
    outFileString = argv[2];

    inFilePtr = fopen(inFileString, "r");
    if (inFilePtr == NULL) {
        printf("error in opening %s\n", inFileString);
        exit(1);
    }

    // Check for blank lines in the middle of the code.
    checkForBlankLinesInCode(inFilePtr);

    outFilePtr = fopen(outFileString, "w");
    if (outFilePtr == NULL) {
        printf("error in opening %s\n", outFileString);
        exit(1);
    }

    // Holder for the array of labels
    char *labels[1000] = {NULL};
    // Holder for the current position in the labels array
    uint32_t pos = 0;
    // First loop, get the labels and their addresses
    while( readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
        // If a label exists on this line, store it with its address
        if(label[0]!= '\0') {
            // Check for duplicates in previous labels
            for (uint32_t i = 0; i < pos; i++) {
                // If labels[i] is NULL, skip it
                if (labels[i] == NULL) {
                    continue;;
                }
                // If the label already exists, throw an error
                else if (strcmp(labels[i], label) == 0) {
                    printf("error: duplicate label '%s'\n", label);
                    exit(1);
                }
            }
            // Check if labels exceed 6 characters
            if (strlen(label) > 6) {
                printf("error: label '%s' is too long\n", label);
                exit(1);
            }
            // Check if the first character is a letter
            if (!isalpha(label[0])) {
                printf("error: label '%s' must start with a letter\n", label);
                exit(1);
            }
            // Store the label in the labels array
            labels[pos] = malloc(strlen(label) + 1);
            strcpy(labels[pos], label);
        }
        pos++;
    }

    // Second loop, translate assembly to machine code
    rewind(inFilePtr);

    // Second loop, translate assembly to machine code
    // Holder for the binary code assembled from one line of assembly code.
    int machine_code;
    pos = 0;
    while (readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2))
    {
        // If a label exists on this line, store it with its address
        if (strcmp(opcode, "halt") == 0 || strcmp(opcode, "noop") == 0 || strcmp(opcode, ".fill") == 0) {
            // Do stuff for halt
            if(strcmp(opcode, "halt") == 0){
                machine_code = 6;
                machine_code <<= 22;
                printHexToFile(outFilePtr, machine_code);
                pos++;
            } else if (strcmp(opcode, "noop") == 0){
                machine_code = 7;
                machine_code <<= 22;
                printHexToFile(outFilePtr, machine_code);
                pos++;
            } else {
                // Check if arg0 is a label
                // Store the label address if it exists
                int address = 0;
                if (!isdigit(arg0[0]) && arg0[0] != '-') {
                    //exit 1 check
                    bool label_found = false;
                    for (uint32_t i = 0; i < 1000; i++) {
                        if (labels[i] != NULL && strcmp(labels[i], arg0) == 0) {
                            label_found = true;
                            address = i;
                            break;
                        }
                    }
                    //exit 1 check: should keep this since test1.as has a .fill with an undefined label
                    if (!label_found) {
                        printf("error: undefined label '%s'\n", arg0);
                        exit(1);
                    }
                    // Store the address in machine_code
                    machine_code = 0;
                    machine_code |= address;
                    printHexToFile(outFilePtr, machine_code);
                    pos++;
                // Check if offset fits in 16 bits
                } else {
                    int offset = atoi(arg0);
                    // Store the offset in machine_code
                    machine_code = 0;
                    machine_code = offset;
                    printHexToFile(outFilePtr, machine_code);
                    pos++;
                }
            }
        } else {
            // Check for Non-integer register arguments for Reg A and B
            // Check for negative register arguments for Reg A and B
            if (!isdigit(arg0[0]) || !isdigit(arg1[0])) {
                printf("error: non-integer register\n");
                exit(1);
            }
            // Check registers outside the range [0, 7] 
            if (atoi(arg0) > 7 || atoi(arg1) > 7) {
                printf("error: register out of range\n");
                exit(1);
            }
            // add and nor
            if (strcmp(opcode, "add") == 0 || strcmp(opcode, "nor") == 0 ){
                if(strcmp(opcode, "add") == 0){
                    machine_code = 0;
                } else {
                    machine_code = 1;
                }
                machine_code <<= 22;
                machine_code |= (atoi(arg0) << 19);
                machine_code |= (atoi(arg1) << 16);
                if (!isNumber(arg2)) {
                    printf("Non-integer register arguments %s\n", arg2);
                    exit(1);
                }
                int num = atoi(arg2);
                if(num < 0 || num > 7) {
                    printf("error: register out of range [0,7]\n");
                    exit(1);
                }
                machine_code |= atoi(arg2);
                printHexToFile(outFilePtr, machine_code);
                pos++;
            // lw, sw, beq
            } else if (strcmp(opcode, "lw") == 0 || strcmp(opcode, "sw") == 0 ||
                    strcmp(opcode, "beq") == 0) {
                if (strcmp(opcode, "lw") == 0) {
                    machine_code = 2;
                } else if (strcmp(opcode, "sw") == 0) {
                    machine_code = 3;
                } else {
                    machine_code = 4;
                }
                machine_code <<= 22;
                machine_code |= (atoi(arg0) << 19);
                machine_code |= (atoi(arg1) << 16);

                // Check if arg2 is a label
                // Store the label address if it exists
                int address;
                int offset;
                if (!isdigit(arg2[0]) && arg2[0] != '-') {
                    bool label_found = false;
                    for (uint32_t i = 0; i < 1000; i++) {
                        if (labels[i] != NULL && strcmp(labels[i], arg2) == 0) {
                            label_found = true;
                            address = i;
                            break;
                        }
                    }
                    if (!label_found) {
                        printf("error: undefined label '%s'\n", arg2);
                        exit(1);
                    }
                    if (strcmp(opcode, "lw") == 0 || strcmp(opcode, "sw") == 0) {
                        machine_code |= (address & 0xFFFF);
                    } else {
                        // For beq, the offset is relative to the next instruction
                        int16_t relative_offset = address - (pos + 1);
                        if (relative_offset < -32768 || relative_offset > 32767) {
                            printf("error: beq offset out of range\n");
                            exit(1);
                        }
                        machine_code |= (relative_offset & 0xFFFF);
                    }
                // Numeric address, dont need counter
                // Check if offset fits in 16 bits
                } else {
                    offset = atoi(arg2);
                    if (offset < -32768 || offset > 32767) {
                        printf("error: .fill value out of range\n");
                        exit(1);
                    }
                    machine_code |= (offset & 0xFFFF);
                }
                printHexToFile(outFilePtr, machine_code);
                pos++;
            // jalr
            } else if (strcmp(opcode, "jalr") == 0) {
                machine_code = 5;
                machine_code <<= 22;
                machine_code |= (atoi(arg0) << 19);
                machine_code |= (atoi(arg1) << 16);
                printHexToFile(outFilePtr, machine_code);
                pos++;
            // Unrecognized opcode
            } else {
                printf("error: unrecognized opcode '%s'\n", opcode);
                exit(1);
            }
        }
    }
    
    
    // /* here is an example for how to use readAndParse to read a line from
    //     inFilePtr */
    // if (! readAndParse(inFilePtr, label, opcode, arg0, arg1, arg2) ) {
    //     /* reached end of file */
    // }

    // /* this is how to rewind the file ptr so that you start reading from the
    //     beginning of the file */
    // rewind(inFilePtr);

    // /* after doing a readAndParse, you may want to do the following to test the
    //     opcode */
    // if (!strcmp(opcode, "add")) {
    //     /* do whatever you need to do for opcode "add" */
    // }

    // /* here is an example of using isNumber. "5" is a number, so this will
    //    return true */
    // if(isNumber("5")) {
    //     printf("It's a number\n");
    // }

    // /* here is an example of using printHexToFile. This will print a
    //    machine code word / number in the proper hex format to the output file */
    // printHexToFile(outFilePtr, 123);

    return(0);
}

// Returns non-zero if the line contains only whitespace.
static int lineIsBlank(char *line) {
    char whitespace[4] = {'\t', '\n', '\r', ' '};
    int nonempty_line = 0;
    for(int line_idx=0; line_idx < strlen(line); ++line_idx) {
        int line_char_is_whitespace = 0;
        for(int whitespace_idx = 0; whitespace_idx < 4; ++ whitespace_idx) {
            if(line[line_idx] == whitespace[whitespace_idx]) {
                line_char_is_whitespace = 1;
                break;
            }
        }
        if(!line_char_is_whitespace) {
            nonempty_line = 1;
            break;
        }
    }
    return !nonempty_line;
}

// Exits 2 if file contains an empty line anywhere other than at the end of the file.
// Note calling this function rewinds inFilePtr.
static void checkForBlankLinesInCode(FILE *inFilePtr) {
    char line[MAXLINELENGTH];
    int blank_line_encountered = 0;
    int address_of_blank_line = 0;
    rewind(inFilePtr);

    for(int address = 0; fgets(line, MAXLINELENGTH, inFilePtr) != NULL; ++address) {
        // Check for line too long
        if (strlen(line) >= MAXLINELENGTH-1) {
            printf("error: line too long\n");
            exit(1);
        }

        // Check for blank line.
        if(lineIsBlank(line)) {
            if(!blank_line_encountered) {
                blank_line_encountered = 1;
                address_of_blank_line = address;
            }
        } else {
            if(blank_line_encountered) {
                printf("Invalid Assembly: Empty line at address %d\n", address_of_blank_line);
                exit(2);
            }
        }
    }
    rewind(inFilePtr);
}


/*
* NOTE: The code defined below is not to be modifed as it is implimented correctly.
*/

/*
 * Read and parse a line of the assembly-language file.  Fields are returned
 * in label, opcode, arg0, arg1, arg2 (these strings must have memory already
 * allocated to them).
 *
 * Return values:
 *     0 if reached end of file
 *     1 if all went well
 *
 * exit(1) if line is too long.
 */
int
readAndParse(FILE *inFilePtr, char *label, char *opcode, char *arg0,
    char *arg1, char *arg2)
{
    char line[MAXLINELENGTH];
    char *ptr = line;

    /* delete prior values */
    label[0] = opcode[0] = arg0[0] = arg1[0] = arg2[0] = '\0';

    /* read the line from the assembly-language file */
    if (fgets(line, MAXLINELENGTH, inFilePtr) == NULL) {
	/* reached end of file */
        return(0);
    }

    /* check for line too long */
    if (strlen(line) == MAXLINELENGTH-1) {
	printf("error: line too long\n");
	exit(1);
    }

    // Ignore blank lines at the end of the file.
    if(lineIsBlank(line)) {
        return 0;
    }

    /* is there a label? */
    ptr = line;
    if (sscanf(ptr, "%[^\t\n ]", label)) {
	/* successfully read label; advance pointer over the label */
        ptr += strlen(label);
    }

    /*
     * Parse the rest of the line.  Would be nice to have real regular
     * expressions, but scanf will suffice.
     */
    sscanf(ptr, "%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]%*[\t\n\r ]%[^\t\n\r ]",
        opcode, arg0, arg1, arg2);

    return(1);
}

static inline int
isNumber(char *string)
{
    int num;
    char c;
    return((sscanf(string, "%d%c",&num, &c)) == 1);
}


// Prints a machine code word in the proper hex format to the file
static inline void printHexToFile(FILE *outFilePtr, int word) {
    fprintf(outFilePtr, "0x%08X\n", word);
}
