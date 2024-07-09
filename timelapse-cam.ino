#include <WiFiClientSecure.h>
#include <esp_crt_bundle.h>
#include <ssl_client.h>

#include <TelegramCertificate.h>
#include <UniversalTelegramBot.h>

#include <FB_Const.h>
#include <FB_Error.h>
#include <FB_Network.h>
#include <FB_Utils.h>
#include <Firebase.h>
#include <FirebaseFS.h>
#include <Firebase_ESP_Client.h>
#include <addons/TokenHelper.h>

#include <FS.h>
#include <LittleFS.h>
#include <M5TimerCAM.h>
#include <WiFi.h>

#include <time.h>

#define WIFI_SSID "YOUR_WIFI_SSID_HERE"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD_HERE"
#define MAX_WIFI_WAIT_SEC 30

#define FIREBASE_USER_EMAIL "YOUR_FIREBASE_AUTH_EMAIL_HERE"
#define FIREBASE_USER_PASSWD "YOUR_FIREBASE_AUTH_PASSWORD_HERE"
#define FIREBASE_API_KEY "YOUR_FIREBASE_API_KEY_HERE"
#define FIREBASE_BUCKET_ID "YOUR_FIREBASE_BUCKET_ID_HERE"

#define TELEGRAM_BOT_TOKEN "YOUR_TELEGRAM_BOT_TOKEN_HERE"
#define TELEGRAM_CHAT_ID "YOUR_TELEGRAM_CHAT_ID_HERE"

#define CAMERA_PIC_FORMAT PIXFORMAT_JPEG
#define CAMERA_VFLIP 1
#define CAMERA_HFLIP 0

#define FIREBASE_PICS_FOLDER "/pics/"
#define PIC_PATH_PREFIX "pic-"
#define MAX_PIC_PATH_LEN 256
#define LOCAL_PIC_PATH "/pic.jpeg"
#define SLEEP_HOURS 6

#define CAM_EXT_WAKEUP_PIN 4

#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC 3600
#define DAYLIGHT_OFFSET_SEC 3600

WiFiClientSecure wifi_client;
UniversalTelegramBot bot(TELEGRAM_BOT_TOKEN, wifi_client);

FirebaseData fb_data;
FirebaseAuth fb_auth;
FirebaseConfig fb_config;

char file_name[MAX_PIC_PATH_LEN];

void setup() {
  Serial.begin(115200);
  TimerCAM.begin(true);
  connect_wifi();
  get_current_time();
  take_picture();
  TimerCAM.Power.setLed(255);
  setup_firebase();
  make_pic_file_name();
  upload_picture();
  send_telegram_message();
  TimerCAM.Camera.deinit();
  TimerCAM.Power.setLed(0);
  TimerCAM.Power.timerSleep(SLEEP_HOURS * 3600);
}

void connect_wifi() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  wifi_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
  Serial.print("Connecting to Wi-Fi");
  int start_time = millis();
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(300);
    if (millis() - start_time >= 1000 * MAX_WIFI_WAIT_SEC)
      TimerCAM.Power.timerSleep(1);
  }
  Serial.println();
  Serial.print("Connected with IP ");
  Serial.println(WiFi.localIP());
  Serial.println();
}

void get_current_time() {
  Serial.println("Setting RTC time from NTP");
  struct tm timeinfo;
  do {
    configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  } while (!getLocalTime(&timeinfo));
  TimerCAM.Rtc.setDateTime(&timeinfo);
  Serial.println("RTC time set");
}

void take_picture() {
  while (!TimerCAM.Camera.begin()) {
    Serial.println("Camera initialization failed. Retrying...");
    delay(500);
  }
  Serial.println("Camera initialized");
  TimerCAM.Camera.sensor->set_pixformat(TimerCAM.Camera.sensor,
                                        CAMERA_PIC_FORMAT);
  TimerCAM.Camera.sensor->set_vflip(TimerCAM.Camera.sensor, CAMERA_VFLIP);
  TimerCAM.Camera.sensor->set_hmirror(TimerCAM.Camera.sensor, CAMERA_HFLIP);
  TimerCAM.Camera.get();
  while (!TimerCAM.Camera.get()) {
    Serial.println("Couldn't take picture. Retrying...");
    TimerCAM.Camera.free();
  }
  Serial.println("Picture taken");
  LittleFS.begin(true);
  File file = LittleFS.open(LOCAL_PIC_PATH, FILE_WRITE);
  file.write(TimerCAM.Camera.fb->buf, TimerCAM.Camera.fb->len);
  file.close();
  TimerCAM.Camera.free();
  Serial.println("Picture saved in local FS");
}

void setup_firebase() {
  Serial.println("Setting up Firebase connection");
  fb_config.api_key = FIREBASE_API_KEY;
  fb_auth.user.email = FIREBASE_USER_EMAIL;
  fb_auth.user.password = FIREBASE_USER_PASSWD;
  fb_config.token_status_callback = tokenStatusCallback;
  Firebase.begin(&fb_config, &fb_auth);
  Firebase.reconnectWiFi(true);
  Serial.println("Firebase connection ready");
}

void upload_picture() {
  Serial.println("Uploading picture to Firebase");
  while (!Firebase.Storage.upload(&fb_data, FIREBASE_BUCKET_ID, LOCAL_PIC_PATH,
                                  mem_storage_type_flash, file_name,
                                  "image/jpeg", fb_upload_callback)) {
    Serial.print("Uploading failed: ");
    Serial.println(fb_data.errorReason());
    Serial.println("Retrying...");
  }
  Serial.println("Picture uploaded to Firebase");
}

void make_pic_file_name() {
  rtc_datetime_t date_time = TimerCAM.Rtc.getDateTime();
  sprintf(file_name, "%s%s%04d-%02d-%02d-%02d:%02d:%02d.jpeg",
          FIREBASE_PICS_FOLDER, PIC_PATH_PREFIX, date_time.date.year,
          date_time.date.month, date_time.date.date, date_time.time.hours,
          date_time.time.minutes, date_time.time.seconds);
}

void fb_upload_callback(FCS_UploadStatusInfo info) {
  if (info.status == firebase_fcs_upload_status_complete) {
    Serial.println("Upload completed\n");
    FileMetaInfo meta = fb_data.metaData();
    Serial.printf("Download URL: %s\n\n", fb_data.downloadURL().c_str());
  }
}

void send_telegram_message() {
  char msg[512];
  int battery_level = TimerCAM.Power.getBatteryLevel();
  sprintf(msg,
          "Battery level: %d%%.\n"
          "Picture saved with name \"%s\".\n"
          "Will wake up again in %d hour%s.",
          battery_level, file_name, SLEEP_HOURS, SLEEP_HOURS > 1 ? "s" : "");
  bot.sendMessage(TELEGRAM_CHAT_ID, msg, "");
}

void loop() {}
