#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[]) {
    FILE *infile = stdin;
    FILE *outfile = stdout;
    FILE *errorFile = stderr;
    int debug = 1; // Initialize to 1 (on) by default
    char *key = NULL;
    int key_length = 0;
    int key_add = 1; // 1 for addition, -1 for subtraction

    // סריקת ארגומנטים בשורת הפקודה
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "+D") == 0) {
            debug = 1;
        } else if (strcmp(argv[i], "-D") == 0) {
            debug = 0;
        } else if (strncmp(argv[i], "+e", 2) == 0 || strncmp(argv[i], "-e", 2) == 0) {
            key = argv[i] + 2;
            key_length = 0;
            while (key[key_length] != '\0') { // רוצה לשנות לIF
                key_length++;
            }
            key_add = (argv[i][0] == '+') ? 1 : -1;
        } else if (strncmp(argv[i], "-I", 2) == 0) { 
            if (debug) {fprintf(errorFile,"%s\n",argv[i]);}
            infile = fopen(argv[i] + 2, "r"); 
            if (infile == NULL) {
                fprintf(errorFile, "Error: Could not open input file %s\n", argv[i] + 2);
                exit(1); 
            }
        } else if (strncmp(argv[i], "-O", 2) == 0) {
            if (debug) {fprintf(errorFile,"%s\n",argv[i]);}
            outfile = fopen(argv[i] + 2, "w");
            if (outfile == NULL) {
                fprintf(errorFile, "Error: Could not open output file %s\n", argv[i] + 2);
                exit(1);
            }
        }

        if (debug) {
            fprintf(errorFile, "Arg %d: %s\n", i, argv[i]);
        }
    }
   
    int ch;
    int key_index = 0;
    while ((ch = fgetc(infile)) != EOF) {
        if (key != NULL ) {
            int key_value = key[key_index] - '0';
            if (ch >= 'a' && ch <= 'z') {
                ch = ((ch - 'a' + key_add * key_value +26) % 26 ) + 'a';
            } else if (ch >= '0' && ch <= '9') {
                ch = ((ch - '0' + key_add * key_value+ 10) % 10)+ '0';
            }
            key_index = (key_index + 1) % key_length;
        }

        fputc(ch, outfile);
    }

    // סגירת הקבצים
    if (infile != stdin) {
        fclose(infile);
    }
    if (outfile != stdout) {
        fclose(outfile);
    }

    return 0;
}