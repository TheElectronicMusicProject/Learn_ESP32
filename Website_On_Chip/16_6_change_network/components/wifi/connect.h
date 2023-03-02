#ifndef CONNECT_H
#   define CONNECT_H
#   include <stdint.h>
#   include <esp_err.h>

void wifi_init(void);
esp_err_t wifi_connect_sta(const char * p_ssid, const char * p_pass, int32_t timeout);
void wifi_connect_ap(const char * p_ssid, const char * p_pass);
void wifi_disconnect(void);
void wifi_destroy_net(void);

#endif /* CONNECT_H */