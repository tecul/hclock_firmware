#ifndef __EXCEPTIONS__
#define __EXCEPTIONS__

typedef void (*exception_handler)(void);

void construct_exceptions(void);
void start_exceptions(void);
exception_handler register_exception_handler(int exception_nb, exception_handler handler);

#endif
