set(component_srcs "mqtt_manager.c")

idf_component_register(SRCS "${component_srcs}"
                       PRIV_REQUIRES driver mqtt
                       INCLUDE_DIRS "include")
