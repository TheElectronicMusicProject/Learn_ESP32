# How to use

Use `esp_rfc2217_server.py` as follows:
`python esp_rfc2217_server.py -p 4000 COM<X>`

On Docker, use `esptool.py` as follows:
`python esptool.py --port rfc2217://localhost:4000?ign_set_control flash_id`
