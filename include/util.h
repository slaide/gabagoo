#define CHECK(COND,ERRMSG,...){if(!(COND)){fprintf(stderr,"CHECK failed: " ERRMSG __VA_OPT__(,) __VA_ARGS__);exit(EXIT_FAILURE);}}
#define discard (void*)
