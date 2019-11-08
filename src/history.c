#include "history.h"

typedef struct hist_entry {
    char *line;
    char *timestamp;
    histdata_t data;
};