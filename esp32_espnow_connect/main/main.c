#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "ESP32_RESPONDER";

// DİĞER ESP32-C3'ÜN MAC ADRESİ
static uint8_t peer_mac_address[] = {0x34, 0xCD, 0xB0, 0xE9, 0x07, 0xC8};

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

// "world!" gönderildiğinde çalışacak callback
static void esp_now_send_cb(const uint8_t *mac_addr, esp_now_send_status_t status) {
    if (status == ESP_NOW_SEND_SUCCESS) {
        ESP_LOGI(TAG, "'world!' cevabi basariyla gonderildi");
    } else {
        ESP_LOGE(TAG, "'world!' cevabi gonderilemedi");
    }
}

// "hello" mesajı geldiğinde çalışacak callback (YENİ VE DOĞRU TANIM)
static void esp_now_recv_cb(const esp_now_recv_info_t *esp_now_info, const uint8_t *data, int len) {
    struct_message received_message;
    if (len == sizeof(received_message)) {
        memcpy(&received_message, data, sizeof(received_message));
        ESP_LOGI(TAG, "Gelen mesaj: %s", received_message.text);

        // Gelen mesaja karşılık cevap gönder
        struct_message message_to_send;
        strcpy(message_to_send.text, "world!");
        // Cevabı, mesajı gönderen MAC adresine yolla (esp_now_info->src_addr)
        esp_now_send(esp_now_info->src_addr, (uint8_t *)&message_to_send, sizeof(message_to_send));
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

    ESP_LOGI(TAG, "Alici baslatildi. 'hello' mesaji bekleniyor...");
}