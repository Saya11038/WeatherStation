//WiFi
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>

WiFiClientSecure client;

//天気予報
const String host = "api.openweathermap.org";
const String APIKEY = "872c00ac8122600e281076207c8753ec";
char geoLat[20] = "35.0388880";
char geoLon[20] = "135.7785796";
int forecastNum = 4;

//Json 天気予報の応答を格納する
const size_t capacity = 3300; //capacity size can caluclated in https://arduinojson.org/v6/assistant/
DynamicJsonDocument forecaseDoc(capacity);

void setup() {
  Serial.begin(115200); // デバッグ用シリアル通信を開始
  WiFi.begin("SSID", "PASSWORD"); // Wi-FiのSSIDとパスワードを入力

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFiに接続中...");
  }

  Serial.println("WiFiに接続しました！");
}

void loop(){
  GetOpenWeatherMap();
  delay(1000);
}

//httpsのgetをする
bool HttpsGet(String host, String url , String * response ) {

  if (WiFi.status() != WL_CONNECTED)
    return false;

  int code = client.connect( host.c_str(), (uint16_t)443);
  if ( !code ) {
    Serial.println( "サーバーに接続できませんでした" );
    return false;
  }
  Serial.println( "サーバーに接続できました" );

  String req = "";
  req += "GET " + url + " HTTP/1.1\r\n";

  req += "Host: " + host + "\r\n";
  req += "Accept: text/plain\r\n";
  req += "Connection: close\r\n";
  req += "\r\n";
  req += "\0";

  client.print(req);
  client.flush();

  unsigned long timeout = micros();
  while (client.available() == 0) {
    if ( micros() - timeout  > 5000000) {
      Serial.println("応答が無くタイムアウトしました");
      client.stop();
      return false;
    }
  }

  String res;
  while (client.available()) {
    res = client.readStringUntil('\r');
    Serial.print(res);
  }
  *response = res;

  client.stop();
  return true;
}

//OpenWeatherMapから応答を得て結果をパースしてforecaseDocに入れる
bool GetOpenWeatherMap()
{
  //urlの生成
  String url = "/data/2.5/forecast?";
  url += "lat=" + String(geoLat) + "&lon=" + String(geoLon);
  url += "&units=metric&lang=ja";
  if (forecastNum > 0)
    url += "&cnt=" + String(forecastNum);
  url += "&appid=" + APIKEY;
  Serial.println(url);

  //OpenWeatherMapから情報を得る
  String response;
  if ( false == HttpsGet( host, url , &response ) )
    return false;
  Serial.printf("\nresponse:%s\n", response.c_str());

  //jsonドキュメントの作成 make JSON document
  DeserializationError err = deserializeJson(forecaseDoc, response);
  if ( err != DeserializationError::Ok )
  {
    Serial.printf("Deserialization Error:%s\n", err.c_str());
    return false;
  }

  return true;
}
