#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "ESP32_C3_SENDER";

// DİĞER ESP32'NİN MAC ADRESİ
static uint8_t peer_mac_address[] = {0xC8, 0xC9, 0xA3, 0xCF, 0xAF, 0x04};

// Veri yapısı
typedef struct struct_message {
    char text[32];
} struct_message;

// Wi-Fi donanımını başlatan fonksiyon
static void wifi_init(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

// "hello" gönderildiğinde çalışacak callback
static void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS) {
        ESP_LOGE(TAG, "'hello' gonderimi basarisiz");
    }
}

// "world!" mesajı geldiğinde çalışacak callback (YENİ VE DOĞRU TANIM)
static void esp_now_recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int len) {
    struct_message received_message;
    // Gelen verinin boyutunu kontrol etmek her zaman iyi bir pratiktir
    if (len == sizeof(received_message)) {
        memcpy(&received_message, data, sizeof(received_message));
        ESP_LOGI(TAG, "Cevap alindi: %s", received_message.text);
    } else {
        ESP_LOGE(TAG, "Beklenmedik boyutta veri alindi: %d", len);
    }
}

void app_main(void) {
    wifi_init();

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_send_cb(esp_now_send_cb));
    ESP_ERROR_CHECK(esp_now_register_recv_cb(esp_now_recv_cb)); // Hata bu satırdaydı

    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, peer_mac_address, 6);
    peer_info.channel = 0;
    peer_info.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer_info));

    struct_message message_to_send;
    strcpy(message_to_send.text, "hello");

    while (1) {
        ESP_LOGI(TAG, "'hello' gonderiliyor...");
        esp_now_send(peer_mac_address, (uint8_t *)&message_to_send, sizeof(message_to_send));
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}