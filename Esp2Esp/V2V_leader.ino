#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>

const char* ssid = "SSID";        // WiFi SSID
const char* password = "PASSWORD";       // WiFi password
const char* udpAddress = "192.168.1.106";    // Alıcı ESP'nin IP adresi
const int udpPort = 1234;                    // UDP portu
const int ackPort = 4321;                    // Geri bildirim portu

WiFiUDP udp;
WiFiUDP ackUdp;  // Geri bildirim için ayrı UDP nesnesi
WebServer server(80);

// Slider değerlerini tutacak değişkenler
int hiz = 0;
int yavaslama = 0;
int pozisyon = 0;
int konum = 0;
bool ackReceived = false;  // Geri bildirim kontrolü

// Web arayüzü için HTML sayfası
String htmlPage = R"=====(
<html>
    <head>
      <meta charset="utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>V2V Lead</title>
      <style>
        body {
          background-color: #f5f5dc; 
          display: flex;
          justify-content: center;
          align-items: center;
          height: 100vh;
          margin: 0;
          font-family: Arial, sans-serif;
        }

        .container {
          text-align: center;
          background-color: white;
          padding: 20px;
          border-radius: 10px;
          box-shadow: 0px 0px 15px rgba(0, 0, 0, 0.2);
        }

        h1 {
          margin-bottom: 20px;
        }

        label {
          font-size: 18px;
          margin-bottom: 15px; 
          display: block;
        }

        input[type="range"] {
          width: 80%;
          margin: 15px 0;
        }

        button {
          background-color: red;
          color: white;
          border: none;
          padding: 15px 30px;
          font-size: 18px;
          cursor: pointer;
          margin-top: 20px;
          border-radius: 5px;
        }

        button:hover {
          background-color: darkred;
        }

        p {
          margin: 10px 0 30px; 
        }

        #notification {
          visibility: hidden;
          min-width: 200px;
          margin-left: -100px;
          background-color: #4CAF50;
          color: white;
          text-align: center;
          border-radius: 2px;
          padding: 16px;
          position: fixed;
          z-index: 1;
          left: 50%;
          bottom: 30px;
          font-size: 17px;
        }

        #notification.show {
          visibility: visible;
          -webkit-animation: fadein 0.5s, fadeout 0.5s 2.5s;
          animation: fadein 0.5s, fadeout 0.5s 2.5s;
        }

        @-webkit-keyframes fadein {
          from {bottom: 0; opacity: 0;} 
          to {bottom: 30px; opacity: 1;}
        }

        @keyframes fadein {
          from {bottom: 0; opacity: 0;}
          to {bottom: 30px; opacity: 1;}
        }

        @-webkit-keyframes fadeout {
          from {bottom: 30px; opacity: 1;} 
          to {bottom: 0; opacity: 0;}
        }

        @keyframes fadeout {
          from {bottom: 30px; opacity: 1;}
          to {bottom: 0; opacity: 0;}
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>V2V Lead</h1>

        <label for="hiz">Ego Speed (0-90):</label>
        <input type="range" id="hiz" name="hiz" min="0" max="90" value="0" oninput="updateSlider(this)">
        <p><span id="hizValue">0</span> km/h</p>

        <label for="yavaslama">Ego Deceleration (0-10):</label>
        <input type="range" id="yavaslama" name="yavaslama" min="0" max="10" value="0" oninput="updateSlider(this)">
        <p><span id="yavaslamaValue">0</span> m/s^2</p>

        <label for="pozisyon">Ego Brake Pedal Position (0-100):</label>
        <input type="range" id="pozisyon" name="pozisyon" min="0" max="100" value="0" oninput="updateSlider(this)">
        <p><span id="pozisyonValue">0</span> %</p>

        <label for="konum">Ego Global Position (0-10000):</label>
        <input type="range" id="konum" name="konum" min="0" max="10000" value="0" oninput="updateSlider(this)">
        <p><span id="konumValue">0</span> m</p>

        <button onclick="sendData()">BREAK</button>
      </div>

      <div id="notification">Mesaj başarıyla alındı</div>

      <script>
        function updateSlider(slider) {
          document.getElementById(slider.name + "Value").innerText = slider.value;
        }
        function sendData() {
          var hiz = document.getElementById('hiz').value;
          var yavaslama = document.getElementById('yavaslama').value;
          var pozisyon = document.getElementById('pozisyon').value;
          var konum = document.getElementById('konum').value;
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/send?hiz=" + hiz + "&yavaslama=" + yavaslama + "&pozisyon=" + pozisyon + "&konum=" + konum, true);
    
          xhr.onload = function() {
            if (xhr.status == 200) {
              showNotification(); 
      }
    };
    
    xhr.send();
  }

      function showNotification() {
        var notification = document.getElementById("notification");
        notification.className = "show";
        setTimeout(function() {
          notification.className = notification.className.replace("show", ""); 
        }, 3000);  // Bildirim 3 saniye sonra kaybolur
      }
      </script>
    </body>
</html>
)=====";

// Verilerin gönderilmesi işlemi ve geri bildirim bekleme
void sendData(int hiz, int yavaslama, int pozisyon, int konum) {
  char message[100];  // Mesajı tutacak buffer
  sprintf(message, "Hız:%d,Yavaşlama:%d,Pozisyon:%d,Konum:%d", hiz, yavaslama, pozisyon, konum);  // Mesaj formatı
  udp.beginPacket(udpAddress, udpPort);
  udp.write((const uint8_t*)message, strlen(message));  // Byte array olarak gönder
  udp.endPacket();
  
  // Geri bildirim bekle
  unsigned long startTime = millis();
  ackReceived = false;
  while (millis() - startTime < 2000) {  
    int packetSize = ackUdp.parsePacket();
    if (packetSize) {
      char ackMessage[10];
      ackUdp.read(ackMessage, 10);
      if (strcmp(ackMessage, "ACK") == 0) {
        ackReceived = true;
        break;
      }
    }
  }
  
  if (ackReceived) {
    Serial.println("Mesaj başarıyla alındı.");
    server.sendContent("<script>showNotification();</script>");
  } else {
    Serial.println("Mesaj alıcıya ulaşmadı.");
  }
}

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleSend() {
  hiz = server.arg("hiz").toInt();
  yavaslama = server.arg("yavaslama").toInt();
  pozisyon = server.arg("pozisyon").toInt();
  konum = server.arg("konum").toInt();
  
  // Verileri alıcıya gönder
  sendData(hiz, yavaslama, pozisyon, konum);
  
  // Tarayıcıya başarı yanıtı gönder
  server.send(200, "text/plain", "OK");
}

void setup() {
  Serial.begin(115200);

  // WiFi'ye bağlan
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi'ye bağlanılıyor...");
  }
  Serial.println("WiFi'ye bağlanıldı.");
  Serial.println(WiFi.localIP());

  udp.begin(udpPort);
  ackUdp.begin(ackPort);  // Geri bildirim için ayrı portu başlat

  // Web server ayarları
  server.on("/", handleRoot);
  server.on("/send", handleSend);
  server.begin();
}

void loop() {
  server.handleClient();
}
