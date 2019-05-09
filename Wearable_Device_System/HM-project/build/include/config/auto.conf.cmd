deps_config := \
	/Users/marciofernandescalil/esp/esp-idf/components/app_trace/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/aws_iot/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/bt/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/driver/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/esp32/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/esp_adc_cal/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/esp_http_client/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/ethernet/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/fatfs/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/freertos/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/heap/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/http_server/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/libsodium/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/log/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/lwip/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/mbedtls/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/mdns/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/mqtt/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/openssl/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/pthread/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/spi_flash/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/spiffs/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/tcpip_adapter/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/vfs/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/wear_levelling/Kconfig \
	/Users/marciofernandescalil/esp/esp-idf/components/bootloader/Kconfig.projbuild \
	/Users/marciofernandescalil/esp/esp-idf/components/esptool_py/Kconfig.projbuild \
	/Users/marciofernandescalil/Documents/Master/_IT_HEART_PROJECT_/Source/HM-project/main/Kconfig.projbuild \
	/Users/marciofernandescalil/esp/esp-idf/components/partition_table/Kconfig.projbuild \
	/Users/marciofernandescalil/esp/esp-idf/Kconfig

include/config/auto.conf: \
	$(deps_config)

ifneq "$(IDF_CMAKE)" "n"
include/config/auto.conf: FORCE
endif

$(deps_config): ;
