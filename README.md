This is a simple Arduino sketch for the [M5Stack Timer Camera](https://docs.m5stack.com/en/unit/timercam). Every time the ESP32 wakes up, it takes a picture, uploads it to a Firebase storage DB, and sends a Telegram message indicating the picture filename and battery level. It then goes into deep sleep for a specified number of hours to minimize power usage.

Required libraries:
- [TimerCam-arduino](https://github.com/m5stack/TimerCam-arduino)
- [Firebase-ESP-Client](https://github.com/mobizt/Firebase-ESP-Client)
- [Universal-Arduino-Telegram-Bot](https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot)

You must set the following constants to the credentials for your WiFi, Firebase bucket and Telegram bot:

- `WIFI_SSID`
- `WIFI_PASSWORD`
- `FIREBASE_USER_EMAIL`
- `FIREBASE_USER_PASSWD`
- `FIREBASE_API_KEY`
- `FIREBASE_BUCKET_ID`
- `TELEGRAM_BOT_TOKEN`
- `TELEGRAM_CHAT_ID`
