/*
    Author: HieuP25
    TikTok: @Hieup25

    CODE mang tính chất tham khảo, tất nhiên chưa tối ưu
    nhiều đoạn bị lặp, điều kiện chưa chặt chẽ, chưa khoa học (kiểu code smell ý :#).
    Mình lười quá, mọi người nhã hứng code cho đẹp nhấ :3 :3
*/

/*
   Typical pin layout used:
   ---------------------------------
               MFRC522      ESP
               Reader/PCD   8266
   Signal      Pin          Pin
   ---------------------------------
   RST/Reset   RST          4 (D2)
   SPI SS      SDA(SS)      5 (D1)
   SPI MOSI    MOSI         D7
   SPI MISO    MISO         D6
   SPI SCK     SCK          D5
*/
/* Chương trình chỉ sử dụng sector 3 để đọc và lưu dữ liệu từ các block:
  sector 4, trailer 19, keyA = 0xFF
  - block 12: Ho va Ten;
  - block 13: so cccd
  - block 14: bien so xe
*/
#include <SPI.h>
#include <MFRC522.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include "web.h"
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

#define SECTOR        4   // Chọn sector 4 để thực hiện việc đọc/ghi dữ liệu
#define NAME_BLOCK    16  // Chọn block thứ 12 (thuộc sector 3) đọc/ghi họ tên 
#define CCCD_BLOCK    17  // Chọn block thứ 13 (thuộc sector 3) đọc/ghi căn cước
#define PLATES_BLOCK  18  // Chọn block thứ 14 (thuộc sector 3) đọc/ghi biển số xe
#define RST_PIN       4   // Chọn chân pin RST trên ESP kết nối đến chân RST trên module RFID
#define SS_PIN        5   // Chọn chân pin SDA trên ESP kết nối đến chân SDA trên module RFID

#define URL_mDNS      "hieup25"

const char *ssid = "TP-Link_0D5E";
const char *password = "93853767";
typedef struct
{
  char uid[16];
  char name[16]; // block 16
  char cccd[16]; // block 17
  char plates[16]; // block 18

} card_info_t;
typedef struct
{
  MFRC522::StatusCode err_code;
  char reason[50];

} msg_response_t;
typedef enum {
  ADD_CARD, SHOW_CARD

} mode_card_t;

int num_client = 0;
mode_card_t g_mode = SHOW_CARD;
// Khởi tạo websocket với port là 81
WebSocketsServer ws = WebSocketsServer(81);
// Khởi tạo server với port là 80
ESP8266WebServer server(80);

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance
MFRC522::MIFARE_Key keyA;
/*Sử dụng để lưu dữ liệu để add card*/
card_info_t g_add_card = {0};
MFRC522::MIFARE_Key g_add_card_key;
/*End*/
void dump_byte_array(byte *buffer, byte bufferSize)
{
  for (byte i = 0; i < bufferSize; i++)
  {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
  }
}
MFRC522::StatusCode auth_sector_with_keyA(byte sector, MFRC522::MIFARE_Key *key)
{
  Serial.println(F("Authenticating using key A..."));
  byte trailerBlock = sector * 4 + 3;
  MFRC522::StatusCode status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, key, &(mfrc522.uid));
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("PCD_Authenticate() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  return status;
}

//MFRC522::StatusCode chg_auth_sector(byte sector, MFRC522::MIFARE_Key *key)
//{
//  Serial.println(F("Authenticating using key A..."));
//  byte trailerBlock = sector * 4 + 3;
//  MFRC522::StatusCode status = (MFRC522::StatusCode)mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_B, trailerBlock, key, &(mfrc522.uid));
//  if (status != MFRC522::STATUS_OK)
//  {
//    Serial.print(F("PCD_Authenticate() failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return status;
//  }
//  byte data[18] = {0};
//  byte size_data = sizeof(data);
//  status = mfrc522.MIFARE_Read((byte)trailerBlock, data, &size_data);
//  if (status != MFRC522::STATUS_OK)
//  {
//    Serial.print(F("MIFARE_Read() failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return status;
//  }
//  // Thay đổi giá trị khóa A và khóa B trong dữ liệu
//  for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++)
//  {
//    data[i] = 0x01;      // Đặt giá trị mới cho khóa A (thay đổi tùy ý)
//    data[10 + i] = 0xAA; // Đặt giá trị mới cho khóa B (thay đổi tùy ý)
//  }
//  // Ghi dữ liệu đã sửa đổi trở lại sector trailerBlock
//  status = mfrc522.MIFARE_Write((byte)trailerBlock, data, (byte)16);
//  if (status != MFRC522::STATUS_OK)
//  {
//    Serial.print(F("MIFARE_Write() failed: "));
//    Serial.println(mfrc522.GetStatusCodeName(status));
//    return status;
//  }
//  return status;
//}
MFRC522::StatusCode get_info_from_card(card_info_t* card_info) {
  MFRC522::StatusCode status;
  byte buffer[18] = {0};
  byte size = sizeof(buffer);
  // Xác thực sector để có quyền đọc data các block
  status = auth_sector_with_keyA(SECTOR, &keyA);
  if (status != MFRC522::STATUS_OK) {
    goto EXIT;
  }
  //get uid
  memset(card_info->uid, 0, 16);
  for (int i = 0; i < mfrc522.uid.size; i++) {
    sprintf(card_info->uid + strlen(card_info->uid), "%02X ", mfrc522.uid.uidByte[i]);
  }
  // get name
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(NAME_BLOCK, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() 'name' failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    goto EXIT;
  }
  snprintf(card_info->name, 16, "%s", buffer);
  if (strlen(card_info->name) == 0)
    strcpy(card_info->name, "NULL");
  // get cccd
  memset(buffer, 0, 18);
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(CCCD_BLOCK, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() 'cccd' failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    goto EXIT;
  }
  snprintf(card_info->cccd, 16, "%s", buffer);
  if (strlen(card_info->cccd) == 0)
    strcpy(card_info->cccd, "NULL");
  // get plates
  memset(buffer, 0, 18);
  status = (MFRC522::StatusCode)mfrc522.MIFARE_Read(PLATES_BLOCK, buffer, &size);
  if (status != MFRC522::STATUS_OK)
  {
    Serial.print(F("MIFARE_Read() 'plates' failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    goto EXIT;
  }
  snprintf(card_info->plates, 16, "%s", buffer);
  if (strlen(card_info->plates) == 0)
    strcpy(card_info->plates, "NULL");
EXIT:
  return status;
}
MFRC522::StatusCode del_info_to_card(byte block, MFRC522::MIFARE_Key* auth_key) {
  MFRC522::StatusCode status;
  byte buffer[18] = {0};
  byte size = sizeof(buffer);
  // Xác thực sector để có quyền xóa block
  status = auth_sector_with_keyA(SECTOR, auth_key);
  if (status != MFRC522::STATUS_OK) {
    return status;
  }
  // xoá, thực tế ghi đè byte 0 vào block
  byte tmp[16] = {0};
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(block, tmp, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_DEL() failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
  }
  return status;
}
void set_info_to_card(card_info_t* card_info, MFRC522::MIFARE_Key* auth_key, msg_response_t* msg_resp) {
  MFRC522::StatusCode status;
  byte buffer[18] = {0};
  byte size = sizeof(buffer);
  // Xác thực sector để có quyền đọc data các block
  status = auth_sector_with_keyA(SECTOR, auth_key);
  if (status != MFRC522::STATUS_OK) {
    strncpy(msg_resp->reason, "authentication Key wrong", 50);
    goto EXIT;
  }
  // set name
  status = del_info_to_card(NAME_BLOCK, auth_key);
  if (status != MFRC522::STATUS_OK) {
    strncpy(msg_resp->reason, "Del block 'name' failed", 50);
    goto EXIT;
  }
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(NAME_BLOCK, (byte*)card_info->name, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() 'name' failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    strncpy(msg_resp->reason, "MIFARE_Write() 'name' failed", 50);
    goto EXIT;
  }
  // set cccd
  status = del_info_to_card(CCCD_BLOCK, auth_key);
  if (status != MFRC522::STATUS_OK) {
    strncpy(msg_resp->reason, "Del block 'cccd' failed", 50);
    goto EXIT;
  }
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(CCCD_BLOCK, (byte*)card_info->cccd, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() 'cccd' failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    strncpy(msg_resp->reason, "MIFARE_Write() 'cccd' failed", 50);
    goto EXIT;
  }
  // set plates
  status = del_info_to_card(PLATES_BLOCK, auth_key);
  if (status != MFRC522::STATUS_OK) {
    strncpy(msg_resp->reason, "Del block 'plates' failed", 50);
    goto EXIT;
  }
  status = (MFRC522::StatusCode) mfrc522.MIFARE_Write(PLATES_BLOCK, (byte*)card_info->plates, 16);
  if (status != MFRC522::STATUS_OK) {
    Serial.print(F("MIFARE_Write() 'plates' failed: "));
    Serial.println(mfrc522.GetStatusCodeName(status));
    strncpy(msg_resp->reason, "MIFARE_Write() 'plates' failed", 50);
    goto EXIT;
  }
EXIT:
  msg_resp->err_code = status;
}
void show_info_from_card(card_info_t* card) {
  String out;
  int size_msg = strlen("type") + strlen("card_info")      +
                 strlen("card_uid") + strlen("name")       +
                 strlen("cccd") + strlen("license_plate")  +
                 sizeof(*card) + 30; // 30 là các ký tự trong bản tin json như :, "" và {}
  Serial.print(F("Size of msg: "));
  Serial.println(size_msg);
  DynamicJsonDocument doc(size_msg);
  doc["type"] = "card_info";
  doc["card_uid"] = card->uid;
  doc["name"] = card->name;
  doc["cccd"] = card->cccd;
  doc["license_plate"] = card->plates;
  serializeJson(doc, out);
  ws.broadcastTXT(out);
}
void WebSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      num_client--;
      Serial.printf("[%u] Disconnected!\n", num);
      break;
    case WStype_CONNECTED:
      {
        num_client++;
        IPAddress ip = ws.remoteIP(num);
        Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
        break;
      }
    case WStype_TEXT:
      {
        Serial.printf("[%u] get Text: %s\n", num, payload);
        DynamicJsonDocument doc(200);
        deserializeJson(doc, payload);
        // Giả sử bản tin đầy đủ
        // Có thể check thêm các điều kiện như containsKey
        // Tránh crash esp
        if (doc.containsKey("type")) {
          const char* _type = doc["type"].as<const char*>();
          if (strcmp(_type, "add_card") == 0) {
            const char* _name = doc["name"].as<const char*>();
            strncpy(g_add_card.name, _name, 16);
            const char* _cccd = doc["cccd"].as<const char*>();
            strncpy(g_add_card.cccd, _cccd, 16);
            const char* _license = doc["license"].as<const char*>();
            strncpy(g_add_card.plates, _license, 16);
            const char* _auth_key = doc["authenticationKey"].as<const char*>();
            // Chỗ này cần kiểm tra truyền đúng format key gồm 6 byte
            // Ở đây, coi như người dùng truyền đúng định dạng từ web, chỗ này cần check đúng định dạng nha. QUAN TRỌNG đó
            char _auth_key_tmp[6][3] = {0};
            sscanf(_auth_key, "%s-%s-%s-%s-%s-%s", _auth_key_tmp[0], _auth_key_tmp[1],
                   _auth_key_tmp[2], _auth_key_tmp[3],
                   _auth_key_tmp[4], _auth_key_tmp[5]);
            for (int i = 0; i < MFRC522::MF_KEY_SIZE; i++) {
              g_add_card_key.keyByte[i] = strtol(_auth_key_tmp[i], NULL, 16);
            }
            Serial.println("SHOW KEY: ");
            dump_byte_array(g_add_card_key.keyByte, 6);
            // Hiển thị lên web là đưa thẻ lại gần
            String out;
            DynamicJsonDocument doc(20);
            doc["type"] = "other";
            serializeJson(doc, out);
            ws.broadcastTXT(out);
            g_mode = ADD_CARD;
          }
        }
        break;
      }
    case WStype_BIN:
    case WStype_ERROR:
    case WStype_FRAGMENT_TEXT_START:
    case WStype_FRAGMENT_BIN_START:
    case WStype_FRAGMENT:
    case WStype_FRAGMENT_FIN:
      break;
  }
}

void setup()
{
  Serial.begin(9600); // Initialize serial communications with the PC
  while (!Serial);                 // Do nothing if no serial port is opened (added for Arduinos based on ATMEGA32U4)
  SPI.begin();        // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522
  delay(4);           // Optional delay. Some board do need more time after init to be ready, see Readme
  // Đặt chế độ Station cho ESP8266,
  // Router đang ở chế độ AP (điểm truy cập)
  WiFi.mode(WIFI_STA);
  // Setup WebServer
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println(F("WiFi connected"));
  Serial.print(F("Camera at address 1: http://"));
  Serial.println(WiFi.localIP());
  Serial.print(F("Camera at address 2: http://"));
  Serial.println(String(URL_mDNS) + ".local");
  // Khởi tạo 1 mDNS có tên myserver
  // có thể truy cập bằng http://hieup25.local
  if (MDNS.begin(URL_mDNS)) {
    Serial.println("MDNS responder started");
  }
  server.on("/", []() {
    server.send(200, "text/html", FPSTR(htmlHomePage));
  });
  server.onNotFound([]() {
    server.send(404, "text/plain", "Not Found");
  });
  server.begin();
  // Setup WebSocket
  ws.begin();
  ws.onEvent(WebSocketEvent);
  /* Fill giá trị 0xFF vào mảng 6 giá trị keyA
    Có thể thay đổi giá trị khác nhau cho 6 giá trị này */
  for (byte i = 0; i < MFRC522::MF_KEY_SIZE; i++)
  {
    keyA.keyByte[i] = 0xFF;
  }
  /**/
  mfrc522.PCD_DumpVersionToSerial(); // Show details of PCD - MFRC522 Card Reader details
  Serial.print(F("Using key A:"));
  dump_byte_array(keyA.keyByte, MFRC522::MF_KEY_SIZE);
  Serial.println();
}
void loop()
{
  server.handleClient();
  MDNS.update();
  ws.loop();
  if (!mfrc522.PICC_IsNewCardPresent() || !mfrc522.PICC_ReadCardSerial())
  {
    return;
  }
  // Show some details of the PICC (that is: the tag/card)
  Serial.print(F("Card UID:"));
  dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);
  Serial.println();
  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
  Serial.println(mfrc522.PICC_GetTypeName(piccType));

  // Check for compatibility
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI && piccType != MFRC522::PICC_TYPE_MIFARE_1K && piccType != MFRC522::PICC_TYPE_MIFARE_4K)
  {
    Serial.println(F("This sample only works with MIFARE Classic cards."));
    return;
  }
  // Show the whole sector as it currently is
  Serial.println(F("Current data in sector:"));
  mfrc522.PICC_DumpMifareClassicSectorToSerial(&(mfrc522.uid), &keyA, SECTOR);
  Serial.println();

  if (g_mode == SHOW_CARD) {
    card_info_t card;
    get_info_from_card(&card);
    show_info_from_card(&card);
  } else {
    msg_response_t msg_resp;
    set_info_to_card(&g_add_card, &g_add_card_key, &msg_resp);
    String out;
    char _reason[50] = {0};
    strncpy(_reason, (strlen(msg_resp.reason) > 0) ? msg_resp.reason : "NaN", 50);
    DynamicJsonDocument doc(100);
    doc["type"] = "add_status";
    doc["error"] = (byte) msg_resp.err_code;
    doc["reason"] = _reason;
    serializeJson(doc, out);
    ws.broadcastTXT(out);
    g_mode = SHOW_CARD;
  }
  // Halt PICC
  mfrc522.PICC_HaltA();
  // Stop encryption on PCD
  mfrc522.PCD_StopCrypto1();
  Serial.println("DONE");
}
