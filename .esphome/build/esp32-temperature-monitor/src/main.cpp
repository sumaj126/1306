// Auto generated code by esphome
// ========== AUTO GENERATED INCLUDE BLOCK BEGIN ===========
#include "esphome.h"
using namespace esphome;
using std::isnan;
using std::min;
using std::max;
using namespace time;
using namespace sensor;
using namespace binary_sensor;
using namespace switch_;
using namespace display;
using namespace text_sensor;
static logger::Logger *logger_logger_id;
static web_server_base::WebServerBase *web_server_base_webserverbase_id;
static wifi::WiFiComponent *wifi_wificomponent_id;
static mdns::MDNSComponent *mdns_mdnscomponent_id;
static esphome::ESPHomeOTAComponent *esphome_esphomeotacomponent_id;
static preferences::IntervalSyncer *preferences_intervalsyncer_id;
static safe_mode::SafeModeComponent *safe_mode_safemodecomponent_id;
static web_server::WebServer *web_server_webserver_id;
const uint8_t ESPHOME_WEBSERVER_INDEX_HTML[174] PROGMEM = {60, 33, 68, 79, 67, 84, 89, 80, 69, 32, 104, 116, 109, 108, 62, 60, 104, 116, 109, 108, 62, 60, 104, 101, 97, 100, 62, 60, 109, 101, 116, 97, 32, 99, 104, 97, 114, 115, 101, 116, 61, 85, 84, 70, 45, 56, 62, 60, 108, 105, 110, 107, 32, 114, 101, 108, 61, 105, 99, 111, 110, 32, 104, 114, 101, 102, 61, 100, 97, 116, 97, 58, 62, 60, 47, 104, 101, 97, 100, 62, 60, 98, 111, 100, 121, 62, 60, 101, 115, 112, 45, 97, 112, 112, 62, 60, 47, 101, 115, 112, 45, 97, 112, 112, 62, 60, 115, 99, 114, 105, 112, 116, 32, 115, 114, 99, 61, 34, 104, 116, 116, 112, 115, 58, 47, 47, 111, 105, 46, 101, 115, 112, 104, 111, 109, 101, 46, 105, 111, 47, 118, 50, 47, 119, 119, 119, 46, 106, 115, 34, 62, 60, 47, 115, 99, 114, 105, 112, 116, 62, 60, 47, 98, 111, 100, 121, 62, 60, 47, 104, 116, 109, 108, 62};
const size_t ESPHOME_WEBSERVER_INDEX_HTML_SIZE = 174;
static api::APIServer *api_apiserver_id;
using namespace api;
static StartupTrigger *startuptrigger_id;
static Automation<> *automation_id;
using namespace i2c;
static i2c::IDFI2CBus *i2c_bus0;
using namespace i2c;
static i2c::IDFI2CBus *i2c_bus1;
using namespace json;
static debug::DebugComponent *debug_debugcomponent_id;
static sntp::SNTPComponent *my_time;
static aht10::AHT10Component *aht10_aht10component_id;
static sensor::Sensor *temperature_sensor;
static sensor::OffsetFilter *sensor_offsetfilter_id;
static sensor::Sensor *humidity_sensor;
static sensor::OffsetFilter *sensor_offsetfilter_id_2;
static sensor::Sensor *free_heap;
static gpio::GPIOBinarySensor *pir_sensor;
static binary_sensor::PressTrigger *binary_sensor_presstrigger_id;
static Automation<> *automation_id_2;
static template_::TemplateSwitch *screen_power;
static Automation<> *automation_id_5;
static ssd1306_i2c::I2CSSD1306 *oled_display;
static template_::TemplateTextSensor *system_status;
static globals::GlobalsComponent<bool> *screen_on;
static globals::GlobalsComponent<int> *last_motion_time;
static StatelessLambdaAction<> *lambdaaction_id;
static StatelessLambdaAction<> *lambdaaction_id_2;
static binary_sensor::ReleaseTrigger *binary_sensor_releasetrigger_id;
static Automation<> *automation_id_3;
static StatelessLambdaAction<> *lambdaaction_id_3;
static esp32::ESP32InternalGPIOPin *esp32_esp32internalgpiopin_id;
static StatelessLambdaAction<> *lambdaaction_id_5;
static Automation<> *automation_id_4;
static StatelessLambdaAction<> *lambdaaction_id_4;
#undef yield
#define yield() esphome::yield()
#undef millis
#define millis() esphome::millis()
#undef micros
#define micros() esphome::micros()
#undef delay
#define delay(x) esphome::delay(x)
#undef delayMicroseconds
#define delayMicroseconds(x) esphome::delayMicroseconds(x)
// ========== AUTO GENERATED INCLUDE BLOCK END ==========="

void setup() {
  // ========== AUTO GENERATED CODE BEGIN ===========
  // network:
  //   enable_ipv6: false
  //   min_ipv6_addr_count: 0
  // esphome:
  //   name: esp32-temperature-monitor
  //   on_boot:
  //     - then:
  //         - lambda: !lambda |
  //              初始化屏幕状态
  //             id(screen_on) = true;
  //           type_id: lambdaaction_id
  //       automation_id: automation_id
  //       trigger_id: startuptrigger_id
  //       priority: 600.0
  //   min_version: 2026.2.2
  //   build_path: build\esp32-temperature-monitor
  //   friendly_name: ''
  //   platformio_options: {}
  //   environment_variables: {}
  //   includes: []
  //   includes_c: []
  //   libraries: []
  //   name_add_mac_suffix: false
  //   debug_scheduler: false
  //   areas: []
  //   devices: []
  App.pre_setup("esp32-temperature-monitor", "", false);
  // time:
  // sensor:
  // binary_sensor:
  // switch:
  // display:
  // text_sensor:
  // logger:
  //   level: INFO
  //   id: logger_logger_id
  //   baud_rate: 115200
  //   tx_buffer_size: 512
  //   deassert_rts_dtr: false
  //   task_log_buffer_size: 768
  //   hardware_uart: UART0
  //   logs: {}
  //   runtime_tag_levels: false
  logger_logger_id = new logger::Logger(115200, 512);
  logger_logger_id->create_pthread_key();
  logger_logger_id->init_log_buffer(768);
  logger_logger_id->set_log_level(ESPHOME_LOG_LEVEL_INFO);
  logger_logger_id->set_uart_selection(logger::UART_SELECTION_UART0);
  logger_logger_id->pre_setup();
  logger_logger_id->set_component_source(LOG_STR("logger"));
  App.register_component(logger_logger_id);
  // web_server_base:
  //   id: web_server_base_webserverbase_id
  web_server_base_webserverbase_id = new web_server_base::WebServerBase();
  web_server_base_webserverbase_id->set_component_source(LOG_STR("web_server_base"));
  App.register_component(web_server_base_webserverbase_id);
  web_server_base::global_web_server_base = web_server_base_webserverbase_id;
  // wifi:
  //   manual_ip:
  //     static_ip: 192.168.1.200
  //     gateway: 192.168.1.1
  //     subnet: 255.255.255.0
  //     dns1: 192.168.1.1
  //     dns2: 8.8.8.8
  //   power_save_mode: NONE
  //   reboot_timeout: 15min
  //   id: wifi_wificomponent_id
  //   domain: .local
  //   fast_connect: false
  //   enable_btm: false
  //   enable_rrm: false
  //   passive_scan: false
  //   enable_on_boot: true
  //   post_connect_roaming: true
  //   min_auth_mode: WPA2
  //   networks:
  //     - ssid: jiajia
  //       password: '9812061104'
  //       id: wifi_wifiap_id
  //       priority: 0
  //   use_address: 192.168.1.200
  wifi_wificomponent_id = new wifi::WiFiComponent();
  wifi_wificomponent_id->set_use_address("192.168.1.200");
  wifi_wificomponent_id->init_sta(1);
  {
  wifi::WiFiAP wifi_wifiap_id = wifi::WiFiAP();
  wifi_wifiap_id.set_ssid("jiajia");
  wifi_wifiap_id.set_password("9812061104");
  wifi_wifiap_id.set_manual_ip(wifi::ManualIP{
      .static_ip = network::IPAddress(192, 168, 1, 200),
      .gateway = network::IPAddress(192, 168, 1, 1),
      .subnet = network::IPAddress(255, 255, 255, 0),
      .dns1 = network::IPAddress(192, 168, 1, 1),
      .dns2 = network::IPAddress(8, 8, 8, 8),
  });
  wifi_wifiap_id.set_priority(0);
  wifi_wificomponent_id->add_sta(wifi_wifiap_id);
  }
  wifi_wificomponent_id->set_reboot_timeout(900000);
  wifi_wificomponent_id->set_power_save_mode(wifi::WIFI_POWER_SAVE_NONE);
  wifi_wificomponent_id->set_min_auth_mode(wifi::WIFI_MIN_AUTH_MODE_WPA2);
  wifi_wificomponent_id->set_component_source(LOG_STR("wifi"));
  App.register_component(wifi_wificomponent_id);
  // mdns:
  //   id: mdns_mdnscomponent_id
  //   disabled: false
  //   services: []
  mdns_mdnscomponent_id = new mdns::MDNSComponent();
  mdns_mdnscomponent_id->set_component_source(LOG_STR("mdns"));
  App.register_component(mdns_mdnscomponent_id);
  // ota:
  // ota.esphome:
  //   platform: esphome
  //   password: esp32123
  //   id: esphome_esphomeotacomponent_id
  //   version: 2
  //   port: 3232
  esphome_esphomeotacomponent_id = new esphome::ESPHomeOTAComponent();
  esphome_esphomeotacomponent_id->set_port(3232);
  esphome_esphomeotacomponent_id->set_auth_password("esp32123");
  esphome_esphomeotacomponent_id->set_component_source(LOG_STR("esphome.ota"));
  App.register_component(esphome_esphomeotacomponent_id);
  // preferences:
  //   id: preferences_intervalsyncer_id
  //   flash_write_interval: 60s
  preferences_intervalsyncer_id = new preferences::IntervalSyncer();
  preferences_intervalsyncer_id->set_write_interval(60000);
  preferences_intervalsyncer_id->set_component_source(LOG_STR("preferences"));
  App.register_component(preferences_intervalsyncer_id);
  // safe_mode:
  //   id: safe_mode_safemodecomponent_id
  //   boot_is_good_after: 1min
  //   disabled: false
  //   num_attempts: 10
  //   reboot_timeout: 5min
  safe_mode_safemodecomponent_id = new safe_mode::SafeModeComponent();
  safe_mode_safemodecomponent_id->set_component_source(LOG_STR("safe_mode"));
  App.register_component(safe_mode_safemodecomponent_id);
  if (safe_mode_safemodecomponent_id->should_enter_safe_mode(10, 300000, 60000)) return;
  // web_server:
  //   port: 80
  //   auth:
  //     username: admin
  //     password: admin123
  //   id: web_server_webserver_id
  //   version: 2
  //   enable_private_network_access: true
  //   web_server_base_id: web_server_base_webserverbase_id
  //   include_internal: false
  //   log: true
  //   compression: gzip
  //   css_url: ''
  //   js_url: https:oi.esphome.io/v2/www.js
  web_server_webserver_id = new web_server::WebServer(web_server_base_webserverbase_id);
  web_server_webserver_id->set_component_source(LOG_STR("web_server"));
  App.register_component(web_server_webserver_id);
  web_server_base_webserverbase_id->set_port(80);
  web_server_webserver_id->set_expose_log(true);
  web_server_base_webserverbase_id->set_auth_username("admin");
  web_server_base_webserverbase_id->set_auth_password("admin123");
  web_server_webserver_id->set_include_internal(false);
  // api:
  //   encryption:
  //     key: cD9kxviHRBXh1PHnWBxAaijqiSnP93X7+SdgtSW5nvI=
  //   id: api_apiserver_id
  //   port: 6053
  //   reboot_timeout: 15min
  //   batch_delay: 100ms
  //   custom_services: false
  //   homeassistant_services: false
  //   homeassistant_states: false
  //   listen_backlog: 4
  //   max_connections: 8
  //   max_send_queue: 8
  api_apiserver_id = new api::APIServer();
  api_apiserver_id->set_component_source(LOG_STR("api"));
  App.register_component(api_apiserver_id);
  api_apiserver_id->set_port(6053);
  api_apiserver_id->set_reboot_timeout(900000);
  api_apiserver_id->set_batch_delay(100);
  api_apiserver_id->set_listen_backlog(4);
  api_apiserver_id->set_max_connections(8);
  api_apiserver_id->set_noise_psk({112, 63, 100, 198, 248, 135, 68, 21, 225, 212, 241, 231, 88, 28, 64, 106, 40, 234, 137, 41, 207, 247, 117, 251, 249, 39, 96, 181, 37, 185, 158, 242});
  startuptrigger_id = new StartupTrigger(600.0f);
  startuptrigger_id->set_component_source(LOG_STR("esphome.coroutine"));
  App.register_component(startuptrigger_id);
  automation_id = new Automation<>(startuptrigger_id);
  // i2c:
  //   id: i2c_bus0
  //   sda: 21
  //   scl: 22
  //   frequency: 400000.0
  //   sda_pullup_enabled: true
  //   scl_pullup_enabled: true
  //   scan: true
  i2c_bus0 = new i2c::IDFI2CBus();
  i2c_bus0->set_component_source(LOG_STR("i2c"));
  App.register_component(i2c_bus0);
  i2c_bus0->set_sda_pin(21);
  i2c_bus0->set_sda_pullup_enabled(true);
  i2c_bus0->set_scl_pin(22);
  i2c_bus0->set_scl_pullup_enabled(true);
  i2c_bus0->set_frequency(400000);
  i2c_bus0->set_scan(true);
  // i2c:
  //   id: i2c_bus1
  //   sda: 4
  //   scl: 5
  //   frequency: 400000.0
  //   sda_pullup_enabled: true
  //   scl_pullup_enabled: true
  //   scan: true
  i2c_bus1 = new i2c::IDFI2CBus();
  i2c_bus1->set_component_source(LOG_STR("i2c"));
  App.register_component(i2c_bus1);
  i2c_bus1->set_sda_pin(4);
  i2c_bus1->set_sda_pullup_enabled(true);
  i2c_bus1->set_scl_pin(5);
  i2c_bus1->set_scl_pullup_enabled(true);
  i2c_bus1->set_frequency(400000);
  i2c_bus1->set_scan(true);
  // json:
  //   {}
  // esp32:
  //   board: esp32dev
  //   framework:
  //     type: arduino
  //     version: 3.3.7
  //     sdkconfig_options: {}
  //     log_level: ERROR
  //     advanced:
  //       compiler_optimization: SIZE
  //       enable_idf_experimental_features: false
  //       enable_lwip_assert: true
  //       ignore_efuse_custom_mac: false
  //       ignore_efuse_mac_crc: false
  //       enable_lwip_mdns_queries: true
  //       enable_lwip_bridge_interface: false
  //       enable_lwip_tcpip_core_locking: true
  //       enable_lwip_check_thread_safety: true
  //       disable_libc_locks_in_iram: true
  //       disable_vfs_support_termios: true
  //       disable_vfs_support_select: true
  //       disable_vfs_support_dir: true
  //       freertos_in_iram: false
  //       ringbuf_in_iram: false
  //       heap_in_iram: false
  //       execute_from_psram: false
  //       loop_task_stack_size: 8192
  //       enable_ota_rollback: true
  //       use_full_certificate_bundle: false
  //       include_builtin_idf_components: []
  //       disable_debug_stubs: true
  //       disable_ocd_aware: true
  //       disable_usb_serial_jtag_secondary: true
  //       disable_dev_null_vfs: true
  //       disable_mbedtls_peer_cert: true
  //       disable_mbedtls_pkcs7: true
  //       disable_regi2c_in_iram: true
  //       disable_fatfs: true
  //     components: []
  //     platform_version: https:github.com/pioarduino/platform-espressif32/releases/download/55.03.37/platform-espressif32.zip
  //     source: pioarduino/framework-arduinoespressif32@https:github.com/espressif/arduino-esp32/releases/download/3.3.7/esp32-core-3.3.7.tar.xz
  //   flash_size: 4MB
  //   variant: ESP32
  //   cpu_frequency: 160MHZ
  // debug:
  //   id: debug_debugcomponent_id
  //   update_interval: 60s
  debug_debugcomponent_id = new debug::DebugComponent();
  debug_debugcomponent_id->set_update_interval(60000);
  debug_debugcomponent_id->set_component_source(LOG_STR("debug"));
  App.register_component(debug_debugcomponent_id);
  // time.sntp:
  //   platform: sntp
  //   id: my_time
  //   servers:
  //     - cn.pool.ntp.org
  //   timezone: CST-8
  //   update_interval: 15min
  my_time = new sntp::SNTPComponent({"cn.pool.ntp.org"});
  my_time->set_update_interval(900000);
  my_time->set_component_source(LOG_STR("sntp.time"));
  App.register_component(my_time);
  my_time->set_timezone("CST-8");
  // sensor.aht10:
  //   platform: aht10
  //   i2c_id: i2c_bus1
  //   address: 0x38
  //   temperature:
  //     name: Temperature
  //     id: temperature_sensor
  //     accuracy_decimals: 1
  //     filters:
  //       - offset: -1.0
  //         type_id: sensor_offsetfilter_id
  //     disabled_by_default: false
  //     force_update: false
  //     unit_of_measurement: °C
  //     device_class: temperature
  //     state_class: measurement
  //   humidity:
  //     name: Humidity
  //     id: humidity_sensor
  //     accuracy_decimals: 1
  //     filters:
  //       - offset: 3.0
  //         type_id: sensor_offsetfilter_id_2
  //     disabled_by_default: false
  //     force_update: false
  //     unit_of_measurement: '%'
  //     device_class: humidity
  //     state_class: measurement
  //   update_interval: 60s
  //   id: aht10_aht10component_id
  //   variant: AHT10
  aht10_aht10component_id = new aht10::AHT10Component();
  aht10_aht10component_id->set_update_interval(60000);
  aht10_aht10component_id->set_component_source(LOG_STR("aht10.sensor"));
  App.register_component(aht10_aht10component_id);
  aht10_aht10component_id->set_i2c_bus(i2c_bus1);
  aht10_aht10component_id->set_i2c_address(0x38);
  aht10_aht10component_id->set_variant(aht10::AHT10);
  temperature_sensor = new sensor::Sensor();
  App.register_sensor(temperature_sensor);
  temperature_sensor->set_name("Temperature", 899752953);
  temperature_sensor->set_device_class("temperature");
  temperature_sensor->set_state_class(sensor::STATE_CLASS_MEASUREMENT);
  temperature_sensor->set_unit_of_measurement("\302\260C");
  temperature_sensor->set_accuracy_decimals(1);
  sensor_offsetfilter_id = new sensor::OffsetFilter(-1.0f);
  temperature_sensor->set_filters({sensor_offsetfilter_id});
  aht10_aht10component_id->set_temperature_sensor(temperature_sensor);
  humidity_sensor = new sensor::Sensor();
  App.register_sensor(humidity_sensor);
  humidity_sensor->set_name("Humidity", 3577765598UL);
  humidity_sensor->set_device_class("humidity");
  humidity_sensor->set_state_class(sensor::STATE_CLASS_MEASUREMENT);
  humidity_sensor->set_unit_of_measurement("%");
  humidity_sensor->set_accuracy_decimals(1);
  sensor_offsetfilter_id_2 = new sensor::OffsetFilter(3.0f);
  humidity_sensor->set_filters({sensor_offsetfilter_id_2});
  aht10_aht10component_id->set_humidity_sensor(humidity_sensor);
  // sensor.debug:
  //   platform: debug
  //   free:
  //     name: Free Memory
  //     id: free_heap
  //     disabled_by_default: false
  //     force_update: false
  //     unit_of_measurement: B
  //     icon: mdi:counter
  //     accuracy_decimals: 0
  //     entity_category: diagnostic
  //   debug_id: debug_debugcomponent_id
  free_heap = new sensor::Sensor();
  App.register_sensor(free_heap);
  free_heap->set_name("Free Memory", 2070763131);
  free_heap->set_icon("mdi:counter");
  free_heap->set_entity_category(::ENTITY_CATEGORY_DIAGNOSTIC);
  free_heap->set_unit_of_measurement("B");
  free_heap->set_accuracy_decimals(0);
  debug_debugcomponent_id->set_free_sensor(free_heap);
  // binary_sensor.gpio:
  //   platform: gpio
  //   pin:
  //     number: 13
  //     mode:
  //       input: true
  //       output: false
  //       open_drain: false
  //       pullup: false
  //       pulldown: false
  //     id: esp32_esp32internalgpiopin_id
  //     inverted: false
  //     ignore_pin_validation_error: false
  //     ignore_strapping_warning: false
  //     drive_strength: 20.0
  //   name: 人体感应
  //   id: pir_sensor
  //   device_class: motion
  //   on_press:
  //     - then:
  //         - lambda: !lambda |
  //             id(last_motion_time) = id(my_time).now().timestamp;
  //             id(screen_on) = true;
  //           type_id: lambdaaction_id_2
  //       automation_id: automation_id_2
  //       trigger_id: binary_sensor_presstrigger_id
  //   on_release:
  //     - then:
  //         - lambda: !lambda |
  //             id(last_motion_time) = id(my_time).now().timestamp;
  //           type_id: lambdaaction_id_3
  //       automation_id: automation_id_3
  //       trigger_id: binary_sensor_releasetrigger_id
  //   disabled_by_default: false
  //   use_interrupt: true
  //   interrupt_type: ANY
  pir_sensor = new gpio::GPIOBinarySensor();
  App.register_binary_sensor(pir_sensor);
  pir_sensor->set_name("\344\272\272\344\275\223\346\204\237\345\272\224", 1174465801);
  pir_sensor->set_device_class("motion");
  pir_sensor->set_trigger_on_initial_state(false);
  binary_sensor_presstrigger_id = new binary_sensor::PressTrigger(pir_sensor);
  automation_id_2 = new Automation<>(binary_sensor_presstrigger_id);
  // switch.template:
  //   platform: template
  //   name: 屏幕状态
  //   id: screen_power
  //   turn_on_action:
  //     then:
  //       - lambda: !lambda |
  //           id(screen_on) = true;
  //         type_id: lambdaaction_id_4
  //     trigger_id: trigger_id
  //     automation_id: automation_id_4
  //   turn_off_action:
  //     then:
  //       - lambda: !lambda |
  //           id(screen_on) = false;
  //         type_id: lambdaaction_id_5
  //     trigger_id: trigger_id_2
  //     automation_id: automation_id_5
  //   disabled_by_default: false
  //   restore_mode: ALWAYS_OFF
  //   optimistic: false
  //   assumed_state: false
  screen_power = new template_::TemplateSwitch();
  App.register_switch(screen_power);
  screen_power->set_name("\345\261\217\345\271\225\347\212\266\346\200\201", 1174465801);
  screen_power->set_restore_mode(switch_::SWITCH_ALWAYS_OFF);
  screen_power->set_component_source(LOG_STR("template.switch"));
  App.register_component(screen_power);
  automation_id_5 = new Automation<>(screen_power->get_turn_off_trigger());
  // display.ssd1306_i2c:
  //   platform: ssd1306_i2c
  //   id: oled_display
  //   i2c_id: i2c_bus0
  //   model: SSD1306_128X64
  //   address: 0x3C
  //   lambda: !lambda " 检查是否需要熄屏\nif (id(my_time).now().timestamp - id(last_motion_time)
  //     \ > 60) {\n  id(screen_on) = false;\n}\n\nif (id(screen_on)) {\n   亮屏状态\n  it.clear();\n
  //     \  \n   显示PIR状态图标\n  if (id(pir_sensor).state) {\n     有人状态\n    it.printf(0,
  //     \ 10, \"MOTION\");\n  } else {\n     无人状态\n    it.printf(0, 10, \"NO MOTION\"
  //     );\n  }\n  \n   显示日期\n  char date_str[16];\n  time_t now = id(my_time).now().timestamp;\n
  //     \  struct tm *timeinfo = localtime(&now);\n  strftime(date_str, sizeof(date_str),
  //     \ \"%Y-%m-%d\", timeinfo);\n  it.printf(0, 20, \"%s\", date_str);\n  \n   显示时间\n
  //     \  char time_str[16];\n  strftime(time_str, sizeof(time_str), \"%H:%M:%S\", timeinfo);\n
  //     \  it.printf(0, 35, \"%s\", time_str);\n  \n   显示温湿度\n  char temp_str[20];\n 
  //     \ char hum_str[20];\n  snprintf(temp_str, sizeof(temp_str), \"T: %.1fC\", id(temperature_sensor).state);\n
  //     \  snprintf(hum_str, sizeof(hum_str), \"H: %.1f%%\", id(humidity_sensor).state);\n
  //     \  it.printf(0, 50, \"%s %s\", temp_str, hum_str);\n} else {\n   熄屏状态\n  it.clear();\n
  //     }\n"
  //   auto_clear_enabled: unspecified
  //   brightness: 1.0
  //   contrast: 1.0
  //   flip_x: true
  //   flip_y: true
  //   offset_x: 0
  //   offset_y: 0
  //   invert: false
  //   update_interval: 1s
  oled_display = new ssd1306_i2c::I2CSSD1306();
  oled_display->set_update_interval(1000);
  oled_display->set_component_source(LOG_STR("display"));
  App.register_component(oled_display);
  oled_display->set_auto_clear(true);
  oled_display->set_model(ssd1306_base::SSD1306_MODEL_128_64);
  oled_display->init_brightness(1.0f);
  oled_display->init_contrast(1.0f);
  oled_display->init_flip_x(true);
  oled_display->init_flip_y(true);
  oled_display->init_offset_x(0);
  oled_display->init_offset_y(0);
  oled_display->init_invert(false);
  // text_sensor.template:
  //   platform: template
  //   name: 系统状态
  //   id: system_status
  //   lambda: !lambda |
  //     return {"online"};
  //   update_interval: 60s
  //   disabled_by_default: false
  system_status = new template_::TemplateTextSensor();
  App.register_text_sensor(system_status);
  system_status->set_name("\347\263\273\347\273\237\347\212\266\346\200\201", 1174465801);
  system_status->set_update_interval(60000);
  system_status->set_component_source(LOG_STR("template.text_sensor"));
  App.register_component(system_status);
  system_status->set_template([]() -> esphome::optional<std::string> {
      #line 208 "d:\\ESP32\\ESP32_1306\\esp32-temperature-monitor.yaml"
      return {"online"};
      
  });
  // md5:
  // sha256:
  //   {}
  // socket:
  //   implementation: bsd_sockets
  // web_server_idf:
  //   {}
  // globals:
  //   id: screen_on
  //   type: bool
  //   initial_value: 'true'
  //   restore_value: false
  screen_on = new globals::GlobalsComponent<bool>(true);
  screen_on->set_component_source(LOG_STR("globals"));
  App.register_component(screen_on);
  // globals:
  //   id: last_motion_time
  //   type: int
  //   initial_value: '0'
  //   restore_value: false
  last_motion_time = new globals::GlobalsComponent<int>(0);
  last_motion_time->set_component_source(LOG_STR("globals"));
  App.register_component(last_motion_time);
  oled_display->set_writer([](display::Display & it) -> void {
      #line 148 "d:\\ESP32\\ESP32_1306\\esp32-temperature-monitor.yaml"
       
      if (my_time->now().timestamp - last_motion_time->value() > 60) {
        screen_on->value() = false;
      }
      
      if (screen_on->value()) {
         
        it.clear();
        
         
        if (pir_sensor->state) {
           
          it.printf(0, 10, "MOTION");
        } else {
           
          it.printf(0, 10, "NO MOTION");
        }
        
         
        char date_str[16];
        time_t now = my_time->now().timestamp;
        struct tm *timeinfo = localtime(&now);
        strftime(date_str, sizeof(date_str), "%Y-%m-%d", timeinfo);
        it.printf(0, 20, "%s", date_str);
        
         
        char time_str[16];
        strftime(time_str, sizeof(time_str), "%H:%M:%S", timeinfo);
        it.printf(0, 35, "%s", time_str);
        
         
        char temp_str[20];
        char hum_str[20];
        snprintf(temp_str, sizeof(temp_str), "T: %.1fC", temperature_sensor->state);
        snprintf(hum_str, sizeof(hum_str), "H: %.1f%%", humidity_sensor->state);
        it.printf(0, 50, "%s %s", temp_str, hum_str);
      } else {
         
        it.clear();
      }
      
  });
  oled_display->set_i2c_bus(i2c_bus0);
  oled_display->set_i2c_address(0x3C);
  lambdaaction_id = new StatelessLambdaAction<>([]() -> void {
      #line 9 "d:\\ESP32\\ESP32_1306\\esp32-temperature-monitor.yaml"
       
      screen_on->value() = true;
      
  });
  automation_id->add_actions({lambdaaction_id});
  lambdaaction_id_2 = new StatelessLambdaAction<>([]() -> void {
      #line 112 "d:\\ESP32\\ESP32_1306\\esp32-temperature-monitor.yaml"
      last_motion_time->value() = my_time->now().timestamp;
      screen_on->value() = true;
      
  });
  automation_id_2->add_actions({lambdaaction_id_2});
  binary_sensor_releasetrigger_id = new binary_sensor::ReleaseTrigger(pir_sensor);
  automation_id_3 = new Automation<>(binary_sensor_releasetrigger_id);
  lambdaaction_id_3 = new StatelessLambdaAction<>([]() -> void {
      #line 117 "d:\\ESP32\\ESP32_1306\\esp32-temperature-monitor.yaml"
      last_motion_time->value() = my_time->now().timestamp;
      
  });
  automation_id_3->add_actions({lambdaaction_id_3});
  pir_sensor->set_component_source(LOG_STR("gpio.binary_sensor"));
  App.register_component(pir_sensor);
  esp32_esp32internalgpiopin_id = new esp32::ESP32InternalGPIOPin();
  esp32_esp32internalgpiopin_id->set_pin(::GPIO_NUM_13);
  esp32_esp32internalgpiopin_id->set_drive_strength(::GPIO_DRIVE_CAP_2);
  esp32_esp32internalgpiopin_id->set_flags(gpio::Flags::FLAG_INPUT);
  pir_sensor->set_pin(esp32_esp32internalgpiopin_id);
  pir_sensor->set_interrupt_type(gpio::INTERRUPT_ANY_EDGE);
  lambdaaction_id_5 = new StatelessLambdaAction<>([]() -> void {
      #line 129 "d:\\ESP32\\ESP32_1306\\esp32-temperature-monitor.yaml"
      screen_on->value() = false;
      
  });
  automation_id_5->add_actions({lambdaaction_id_5});
  automation_id_4 = new Automation<>(screen_power->get_turn_on_trigger());
  lambdaaction_id_4 = new StatelessLambdaAction<>([]() -> void {
      #line 126 "d:\\ESP32\\ESP32_1306\\esp32-temperature-monitor.yaml"
      screen_on->value() = true;
      
  });
  automation_id_4->add_actions({lambdaaction_id_4});
  screen_power->set_optimistic(false);
  screen_power->set_assumed_state(false);
  // =========== AUTO GENERATED CODE END ============
  App.setup();
}

void loop() {
  App.loop();
}
