#ifndef DIALER_H
#define DIALER_H

int dialer_open(char const *uri, int num_connections, long deadline);
void dialer_close(void);

#endif
