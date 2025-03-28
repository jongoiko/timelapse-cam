This is a simple Arduino sketch for the [M5Stack Timer Camera](https://docs.m5stack.com/en/unit/timercam) that allows automatically capturing images at a low frame rate, making it useful for [time-lapse photography](https://en.wikipedia.org/wiki/Time-lapse_photography).
- Pictures are uploaded to a Firebase storage DB.
- After taking a picture, the ESP32 sends a Telegram message reporting its current battery level.
- Power consumption is minimized to extend battery life for as long as possible; in my experience, taking a picture every 6 hours required changing the battery only every ~3 weeks. This is achieved by having the ESP32 go into [deep sleep](https://randomnerdtutorials.com/esp32-deep-sleep-arduino-ide-wake-up-sources/) in between frames.

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

Also of interest is `SLEEP_HOURS`, which sets the number of hours in between picture frames.
Note that the true picture frequency varies depending on the precision of the microcontroller's clock.
