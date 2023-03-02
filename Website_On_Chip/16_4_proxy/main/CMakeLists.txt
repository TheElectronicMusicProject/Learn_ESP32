idf_component_register(
    SRCS
        "main.c"
        "pushbtn.c"
        "toggle_led.c"
    INCLUDE_DIRS
        "."
)

spiffs_create_partition_image(
    spiffs
        ../site/build FLASH_IN_PROJECT
)