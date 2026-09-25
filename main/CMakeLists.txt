file(GLOB_RECURSE SOURCES "src/**.cpp" "src/**.c")

if(CONFIG_CM_BUILD_FIRMWARE)
    set(ENTRY "main.cpp")
elseif(CONFIG_CM_EXAMPLE_ANCS)
    set(ENTRY "../examples/ancs.cpp")
elseif(CONFIG_CM_EXAMPLE_IMU)
    set(ENTRY "../examples/IMU.cpp")
elseif(CONFIG_CM_EXAMPLE_ANDROID)
    set(ENTRY "../examples/android.cpp")
elseif(CONFIG_CM_EXAMPLE_GATT_SERVER)
    set(ENTRY "../examples/GATT_Server.cpp")
elseif(CONFIG_CM_EXAMPLE_GYRO)
    file(GLOB_RECURSE ENTRY "../examples/Gyro/**.cpp")
    list(APPEND ENTRY "../examples/Gyro/main.cpp")
elseif(CONFIG_CM_TEST_SLEEP)
    set(ENTRY "../examples/Sleep.cpp")
elseif(CONFIG_CM_TEST_SLEEP_WAKE)
    set(ENTRY "../examples/Sleep_Wake.cpp")
elseif(CONFIG_CM_TEST_SLEEP_WAKE_INT)
    set(ENTRY "../examples/Sleep_Wake_Interrupt.cpp")
endif()

file(GLOB_RECURSE LIBS "lib/*/src/**.cpp" "lib/*/src/**.c")
set(LIBS_INCL "lib/mjson/src" "lib/glm/glm")

idf_component_register(SRCS ${ENTRY} ${SOURCES} ${LIBS} INCLUDE_DIRS "src" ${LIBS_INCL}
        REQUIRES bootloader_support bt driver efuse esp_adc esp_common esp_event
        esp_hw_support esp_netif esp_pm esp_psram esp_system esp_timer esp_wifi heap log
        JPEGDEC LovyanGFX Perse-Common lvgl nvs_flash pthread spiffs)

spiffs_create_partition_image(storage ../spiffs_image FLASH_IN_PROJECT)
