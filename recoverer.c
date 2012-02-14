#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "match.h"

#define BLOCK_SIZE 262144

int main (int argc, char **argv) {
    int i, j;
    FILE *fp, *out;
    short *start_print, *end_print;
    int end_plen;
    int min_size, max_size;
    long long offset;
    int file_no = 0, file_start, file_len;
    int reprocess_p = 0, in_file_p = 0;
    char ext[8];
    char filename[20];
    char raw_print[PRINT_SIZE * 2 + 2];
    char *hex = "0123456789ABCDEF";
    char *s, *data;

    start_print = malloc(PRINT_SIZE *  sizeof(start_print));
    end_print = malloc(PRINT_SIZE *  sizeof(end_print));

    if ( start_print == NULL || end_print == NULL ) {
        return 2;
    }

    if ( argc != 3 ) {
        fprintf(stderr, "Usage: %s <fingerprint> <disk image>\n", argv[0]);
        return 1;
    }

    fp = fopen(argv[1], "r");

    if ( fp == NULL ) {
        fprintf(stderr, "Couldn't open %s\n", argv[1]);
        return 1;
    }

    fgets(ext, 8, fp);
    if ( ( s = strchr(ext, '\n') ) != NULL ) {
        *s = '\0';
    }

    fscanf(fp, "%d\n", &min_size);
    fscanf(fp, "%d\n", &max_size);

    fgets(raw_print, PRINT_SIZE * 2 + 2, fp);

    for ( i = 0 ; i < PRINT_SIZE ; i++ ) {
        if ( raw_print[i * 2] == 'X' ) {
            start_print[i] = -1;
        } else {
            start_print[i] = ( strchr(hex, raw_print[i * 2]) - hex ) * 16 + ( strchr(hex, raw_print[i * 2 + 1]) - hex );
        }
    }

    fgets(raw_print, PRINT_SIZE * 2 + 2, fp);

    for ( i = 0, end_plen = 0 ; i < strlen(raw_print) / 2; i++ ) {
        if ( raw_print[i * 2] == 'X' ) {
            if ( end_plen > 0 ) {
                end_print[end_plen++] = -1;
            }
        } else {
            end_print[end_plen++] = ( strchr(hex, raw_print[i * 2]) - hex ) * 16 + ( strchr(hex, raw_print[i * 2 + 1]) - hex );
        }
    }

/*
    for ( i = 0 ; i < PRINT_SIZE ; i++ ) {
        if ( start_print[i] == -1 ) {
            printf("%2d - Ignored\n", i);
        } else {
            printf("%2d - %02X\n", i, (unsigned char)start_print[i]);
        }
    }

    for ( i = 0 ; i < end_plen; i++ ) {
        if ( end_print[i] == -1 ) {
            printf("%2d - Ignored\n", i);
        } else {
            printf("%2d - %02X\n", i, (unsigned char)end_print[i]);
        }
    }
*/

    fclose(fp);

    // got our patterns, now go through the file...

    fp = fopen(argv[2], "r");

    if ( fp == NULL ) {
        fprintf(stderr, "Couldn't open %s\n", argv[2]);
        return 1;
    }

    data = malloc(BLOCK_SIZE);

    if ( data == NULL ) {
        return 2;
    }

    offset = 0;

    for ( ; ; ) {
        if ( ! reprocess_p ) {
            j = fread(data, sizeof(*data), BLOCK_SIZE, fp);
            s = data;
        } else {
            reprocess_p = 0;
        }

        if ( ! in_file_p ) {
            i = check_pattern_start(s, j, start_print, PRINT_SIZE);

            if ( i >= 0 ) {
                file_start = offset + i;
                file_len = PRINT_SIZE;
                printf("Found pattern start at %d (%d total)\n", offset + i, file_start);
                in_file_p = 1;

                s += i;

                sprintf(filename, "%04d.%s", file_no, ext);
                out = fopen(filename, "w");

                if ( out == NULL ) {
                    fprintf(stderr, "Couldn't open file for writing\n");
                    return 3;
                }

                fwrite(s, sizeof(*s), PRINT_SIZE, out);

                s += PRINT_SIZE;
                j -= i + PRINT_SIZE;
                offset += i;
                reprocess_p = 1;
            } else {
                offset += j;
            }
        } else if ( file_len + j > min_size ) {
            i = check_pattern_end(s, j, end_print, end_plen);

            if ( i >= 0 && file_len + i > min_size ) {
                file_len += i;
                printf("Found pattern end at %d (%d total)\n", offset + i, file_len);

                fwrite(s, sizeof(*s), i, out);
                fclose(out);
                file_no++;

                in_file_p = 0;
                s += i;
                j -= i;
                offset += i;
                reprocess_p = 1;
            } else {
                file_len += j;
                fwrite(s, sizeof(*s), j, out);
                offset += j;

                if ( file_len > max_size ) {
                    printf("Aborting file due to size\n");
                    fclose(out);
                    in_file_p = 0;
                }
            }
        } else {
            offset += j;

            if ( in_file_p ) {
                fwrite(s, sizeof(*s), j, out);
                file_len += j;

                if ( file_len > max_size ) {
                    printf("Aborting file due to size\n");
                    fclose(out);
                    in_file_p = 0;
                }
            }
        }

        if ( feof(fp) && ! reprocess_p ) {
            fclose(fp);
            return 0;
        }
    }
}
