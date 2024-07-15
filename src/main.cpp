#include <Arduino.h>

#include <esp32_smartdisplay.h>
#include <ui/ui.h>

#include <Wire.h>
#include <AHT20.h>
#include <GyverBME280.h>

float br_level = 0.25;

AHT20 aht20(0x38);
void ath20_data_timer(lv_timer_t * timer);

GyverBME280 bmp280;
void bmp280_data_timer(lv_timer_t * timer);

void update_temperature(float temperature);
void update_humidity(float humidity);

void OnAddOneClicked(lv_event_t *e)
{
    static uint8_t cnt = 0;
    cnt++;
    lv_label_set_text_fmt(ui_lblCountValue, "%d", cnt);
}

void OnRotateClicked(lv_event_t *e)
{
    auto disp = lv_disp_get_default();
    auto rotation = (lv_disp_rot_t)((lv_disp_get_rotation(disp) + 1) % (LV_DISP_ROT_270 + 1));
    lv_disp_set_rotation(disp, rotation);
}


// float BrightnessCallback() {
//   float adapt=smartdisplay_lcd_adaptive_brightness_cds();

//   char text_buffer[16];
//   sprintf(text_buffer, "%d %%", (int) round(adapt*100));
//   lv_label_set_text(ui_lblMillisecondsValue, text_buffer);

// //   log_i("Br: %0.2f", adapt);
//   return adapt;
// }

void update_temperature(float temperature) {
    // Ensure temperature is within range
    if (temperature < 0.0f) temperature = 0.0f;
    if (temperature > 50.0f) temperature = 50.0f;

    // Update arc value
    lv_arc_set_value(tempArc, (int)temperature);

    // Format and update labels
    char intPartStr[10];
    char decimalPartStr[5];
    snprintf(intPartStr, sizeof(intPartStr), "%d°", (int)temperature);
    snprintf(decimalPartStr, sizeof(decimalPartStr), ". %1d", (int)((temperature - (int)temperature) * 10));

    lv_label_set_text(tempValueLabelInt, intPartStr);
    lv_label_set_text(tempValueLabelDecimal, decimalPartStr);
}

void update_humidity(float humidity) {
    if (humidity < 0.0f) humidity = 0.0f;
    if (humidity > 100.0f) humidity = 100.0f;

    // Assuming 'humidity_value_label' is the lv_obj_t* for the humidity value label
    // Convert the float humidity value to a string with a '%' character, formatting to one decimal place
    char humidity_str[20]; // Ensure this buffer is large enough for the number, decimal point, and '%' character
    // snprintf(humidity_str, sizeof(humidity_str), "%.1f%%", humidity);
    snprintf(humidity_str, sizeof(humidity_str), "%d%%", (int)humidity);

    // Update the label with the new humidity value
    lv_arc_set_value(humidity_arc, (int)humidity);
    lv_label_set_text(humidity_value_label, humidity_str);
}


void ath20_data_timer(lv_timer_t * timer) {
  if (aht20.available() == true)
  {
    //Get the new temperature and humidity value
    float temperature = aht20.getTemperature();
    float humidity = aht20.getHumidity();

    update_temperature(temperature);
    update_humidity(humidity);

    log_i("Temp ATH: %0.2f", temperature);
    log_i("Hum  ATH: %0.2f", humidity);

    // char text_buffer[16];
    // sprintf(text_buffer, "%0.2f C", temperature);
    // lv_label_set_text(ui_AhtTempValue, text_buffer);

    // sprintf(text_buffer, "%0.1f RH", humidity);
    // lv_label_set_text(ui_AhtHumValue, text_buffer);
  }
}

void bmp280_data_timer(lv_timer_t * timer) {
    float temp = bmp280.readTemperature();
    float pressure = bmp280.readPressure();

    log_i("Temp BMP: %0.2f", temp);
    log_i("Pres BMP: %0.1f", pressure);

    // char text_buffer[16];
    // sprintf(text_buffer, "%0.2f C", temp);
    // lv_label_set_text(ui_BmpTempValue, text_buffer);

    // sprintf(text_buffer, "%0.1f mmHg", pressureToMmHg(pressure));
    // lv_label_set_text(ui_BmpPressValue, text_buffer);
}


void setup()
{
#ifdef ARDUINO_USB_CDC_ON_BOOT
    delay(3000);
#endif
    Serial.begin(115200);
    Serial.setDebugOutput(true);
    log_i("Board: %s", BOARD_NAME);
    log_i("CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    log_i("Free heap: %d bytes", ESP.getFreeHeap());
    log_i("Free PSRAM: %d bytes", ESP.getPsramSize());
    log_i("SDK version: %s", ESP.getSdkVersion());

    smartdisplay_init();
    

    __attribute__((unused)) auto disp = lv_disp_get_default();
    lv_disp_set_rotation(disp, LV_DISP_ROT_90);
    // lv_disp_set_rotation(disp, LV_DISP_ROT_180);
    // lv_disp_set_rotation(disp, LV_DISP_ROT_270);


    smartdisplay_lcd_set_backlight(br_level);
    ui_init();

    // To use third party libraries, enable the define in lv_conf.h: #define LV_USE_QRCODE 1
    // auto ui_qrcode = lv_qrcode_create(ui_scrMain, 100, lv_color_black(), lv_color_white());
    // const char *qr_data = "https://github.com/rzeldent/esp32-smartdisplay";
    // lv_qrcode_update(ui_qrcode, qr_data, strlen(qr_data));
    // lv_obj_center(ui_qrcode);

    // smartdisplay_lcd_set_backlight(br_level);
    // smartdisplay_lcd_set_brightness_cb(BrightnessCallback, 500);


    if (Wire.begin(22, 27)) {
        if (aht20.begin() == false)
        {
            Serial.println("AHT20 not detected. Please check wiring. Freezing.");
            while (1);
        }

        if (!bmp280.begin(0x77)) Serial.println("Error BMP280!");

        static uint32_t user_data = 10;
        lv_timer_create(ath20_data_timer, 7000,  &user_data);
        lv_timer_create(bmp280_data_timer, 5000,  &user_data);
    } else {
        Serial.println("Error starting wire");
    }
    Serial.println("PING 2");
}

ulong next_millis;

void loop()
{
    auto const now = millis();
    if (now > next_millis)
    {
        next_millis = now + 500;

        // Serial.println("PING");

// // #ifdef BOARD_HAS_RGB_LED
// //         // auto const rgb = (now / 2000) % 8;
// //         // smartdisplay_led_set_rgb(rgb & 0x01, rgb & 0x02, rgb & 0x04);
// // #endif

#ifdef BOARD_HAS_CDS
        char text_buffer[16];
        auto cdr = analogReadMilliVolts(CDS);
        sprintf(text_buffer, "%d", cdr);
        lv_label_set_text(ui_lblCdrValue, text_buffer);
#endif
    }

    lv_timer_handler();
}