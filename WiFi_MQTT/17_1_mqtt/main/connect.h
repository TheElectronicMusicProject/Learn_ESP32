#ifndef CONNECT_H
#   define CONNECT_H

#   include <esp_err.h>

esp_err_t wifi_init(void);
void start_wifi(void);
void stop_wifi(void);

#endif /* CONNECT_H */