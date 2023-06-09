idf_component_register(SRCS "simon.c"
                       INCLUDE_DIRS "."
                       REQUIRES "wifi_sta" "driver" "touch_sensor" "mqtt_simon" "json")
