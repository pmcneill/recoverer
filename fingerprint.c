/*
 * fingerprint.c
 * Patrick McNeill (pmcneill@gmail.com)
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "match.h"

#define BLOCK_SIZE 262144

int main (int argc, char **argv) {
    int i, j, offset;
    FILE *fp;
    short *start_print;
    short *end_print;
    int start_good = 0, end_good = 0;
    unsigned char data[PRINT_SIZE];
    long min_size, max_size;
    char ext[8];
    char filename[100];
    char *s;

    start_print = malloc(sizeof(start_print) * PRINT_SIZE);
    end_print = malloc(sizeof(end_print) * PRINT_SIZE);

    if ( ! start_print || ! end_print ) {
        return 2;
    }

    for ( i = 1 ; i < argc ; i++ ) {
        fp = fopen(argv[i], "r");

        if ( ! fp ) {
            printf("%s not found\n", argv[i]);
            continue;
        }

        // check that we actually get the right number of chars here, I guess
        fread(data, 1, PRINT_SIZE, fp);

        if ( i == 1 ) {
            for ( j = 0 ; j < PRINT_SIZE ; j++ ) {
                start_print[j] = data[j];
            }
        } else {
            for ( j = 0 ; j < PRINT_SIZE ; j++ ) {
                if ( start_print[j] != -1 && start_print[j] != data[j] ) {
                    start_print[j] = -1;
                }
            }
        }

        fseek(fp, -PRINT_SIZE, SEEK_END);
        j = fread(data, 1, PRINT_SIZE, fp);

        if ( i == 1 ) {
            for ( j = 0 ; j < PRINT_SIZE ; j++ ) {
                end_print[j] = data[j];
            }
        } else {
            for ( j = 0 ; j < PRINT_SIZE ; j++ ) {
                if ( end_print[j] != -1 && end_print[j] != data[j] ) {
                    end_print[j] = -1;
                }
            }
        }

        fclose(fp);
    }

    printf("Fingerprint:\n");
    printf("Start                      End\n");

    for ( i = 0 ; i < PRINT_SIZE ; i++ ) {
        if ( start_print[i] == -1 ) {
            printf("%2d - Ignored", i);
        } else {
            printf("%2d - %02X     ", i, (unsigned char)start_print[i]);
            start_good++;
        }

        if ( end_print[i] == -1 ) {
            printf("               %2d - Ignored\n", i);
        } else {
            printf("               %2d - %02X\n", i, (unsigned char)end_print[i]);
            end_good++;
        }
    }

    if ( start_good <= 1 || end_good <= 1 ) {
        printf("Error: No enough common data found among file set.\n");
        return 1;
    }

    printf("File extension: ");
    fflush(stdin);
    fgets(ext, 8, stdin);

    if ( ( s = strchr(ext, '\n') ) != NULL ) {
        *s = '\0';
    }

    printf("File type: ");
    fflush(stdin);
    fgets(filename, 90, stdin);

    if ( ( s = strchr(filename, '\n') ) != NULL ) {
        *s = '\0';
    }

    printf("Minimum size: ");
    fflush(stdin);
    scanf("%ld", &min_size);

    printf("Maximum size: ");
    fflush(stdin);
    scanf("%ld", &max_size);

    // Start the test phase.  Reopen each file and check that the end_print
    // is truly distinct after the chosen minimum filesize.  If the match
    // occurs in the last block we pull out, the end pattern probably isn't
    // good anyways.

    s = malloc(BLOCK_SIZE);

    if ( ! s ) {
        return 2;
    }

    for ( i = 1 ; i < argc ; i++ ) {
        int match_pos;

        fp = fopen(argv[i], "r");

        if ( ! fp ) {
            continue;
        }

        j = fread(s, 1, BLOCK_SIZE, fp);
        match_pos = check_pattern_start(s, j, start_print, PRINT_SIZE);

        if ( match_pos != 0 ) {
            printf("%s didn't match at the start .. crap! %d\n", argv[i], match_pos);
            return 2;
        }

        offset = min_size;
        fseek(fp, min_size, SEEK_SET);

        while ( ! feof(fp) ) {
            j = fread(s, 1, BLOCK_SIZE, fp);

            match_pos = check_pattern_end(s, j, end_print, PRINT_SIZE);

            if ( match_pos != -1 && j == BLOCK_SIZE ) {
                printf("%s matched the end pattern before the end of the file at %d!\n", argv[i], offset + match_pos);
                break;
            }

            offset += j;
        }

        fclose(fp);
    }

    strcat(filename, ".prn");

    fp = fopen(filename, "w");

    fprintf(fp, "%s\n%ld\n%ld\n", ext, min_size, max_size);

    for ( i = 0 ; i < PRINT_SIZE ; i++ ) {
        if ( start_print[i] == -1 ) {
            fprintf(fp, "XX");
        } else {
            fprintf(fp, "%02X", start_print[i]);
        }
    }

    fprintf(fp, "\n");

    for ( i = 0 ; i < PRINT_SIZE ; i++ ) {
        if ( end_print[i] == -1 ) {
            fprintf(fp, "XX");
        } else {
            fprintf(fp, "%02X", end_print[i]);
        }
    }

    fprintf(fp, "\n");

    return 0;
}
