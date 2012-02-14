#include <stdio.h>
#include <stdlib.h>

int check_pattern_start(char *data, int len, short *pattern, int plen) {
    int i, j;
    int match;

    for ( i = 0 ; i < ( len - plen ) ; i++ ) {
        match = 1;

        for ( j = 0 ; j < plen ; j++ ) {
            if ( pattern[j] != -1 && \
                 (unsigned char)data[i + j] != (unsigned char)(pattern[j] & 0x00FF) ) {
                match = 0;
                break;
            }
        }

        if ( match ) {
            return i;
        }
    }

    return -1;
}

int check_pattern_end(char *data, int len, short *pattern, int plen) {
    int i, j;
    int match;

    while ( *pattern == -1 && plen > 0 ) {
        pattern++;
        plen--;
    }

    if ( plen == 0 ) {
        exit(3);
    }

    for ( i = 0 ; i < ( len - plen + 1) ; i++ ) {
        match = 1;

        for ( j = 0 ; j < plen ; j++ ) {
            if ( pattern[j] != -1 && \
                 (unsigned char)data[i + j] != (unsigned char)(pattern[j] & 0x00FF) ) {
                match = 0;
                break;
            }
        }

        if ( match ) {
            // return one after the position of the last match
            return i + j;
        }
    }

    return -1;
}
