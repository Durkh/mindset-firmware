idf_component_register(SRCS "actuators.cpp"
                        INCLUDE_DIRS "include"
                        REQUIRES driver LiquidCrystal IRremoteESP8266
                        )