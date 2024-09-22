#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>

const char* ssid = "SSID";      // WiFi SSID
const char* password = "PASSWORD";     // WiFi password
const int udpPort = 1234;                  // Lead cihazdan gelen UDP portu
const int ackPort = 4321;                  // Lead'e geri bildirim gönderilecek port

// Sabit IP bilgileri
IPAddress local_IP(192, 168, 1, 106);      // Sabit Follower IP
IPAddress gateway(192, 168, 1, 1);         // Ağ geçidi
IPAddress subnet(255, 255, 255, 0);        // Subnet maskesi
IPAddress primaryDNS(8, 8, 8, 8);          // DNS sunucusu
IPAddress secondaryDNS(8, 8, 4, 4);        // İkinci DNS sunucusu

IPAddress vericiIP(192, 168, 1, 105);

WiFiUDP udp;
WebServer server(80);

char incomingPacket[255];                  // Gelen UDP verisi için buffer
int hiz = 0;
int yavaslama = 0;
int pozisyon = 0;
int konum = 0;

// Web arayüzü için HTML sayfası
String htmlPage = R"=====( 
<html>
    <head>
      <meta charset="utf-8">
      <meta name="viewport" content="width=device-width, initial-scale=1">
      <title>V2V Follower</title>
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

        p {
          font-size: 18px;
          margin: 10px 0;
        }

        #brakeButton {
          background-color: lightgray;
          color: white;
          padding: 10px 20px;
          font-size: 18px;
          border: none;
          border-radius: 5px;
          cursor: pointer;
          margin-top: 20px;
        }

        #brakeButton.active {
          background-color: red;  
        }
      </style>
    </head>
    <body>
      <div class="container">
        <h1>V2V Follower</h1>
        <p>Ego Speed: <span id="hiz">%HIZ%</span> km/h</p>
        <p>Ego Deceleration: <span id="yavaslama">%YAVASLAMA%</span> m/s²</p>
        <p>Ego Brake Pedal Position: <span id="pozisyon">%POZISYON%</span> %</p>
        <p>Ego Global Position: <span id="konum">%KONUM%</span> m</p>

        <!-- Acil Fren Butonu -->
        <button id="brakeButton">Acil Fren</button>
      </div>

      <script>
        setInterval(function() {
          var xhr = new XMLHttpRequest();
          xhr.open("GET", "/data", true);
          xhr.onload = function() {
            if (xhr.status == 200) {
              var data = xhr.responseText.split(",");
              document.getElementById("hiz").innerText = data[0];
              document.getElementById("yavaslama").innerText = data[1];
              document.getElementById("pozisyon").innerText = data[2];
              document.getElementById("konum").innerText = data[3];

              var brakeButton = document.getElementById("brakeButton");
              var pozisyon = parseInt(data[2]);
              var yavaslama = parseInt(data[1]);
              var hiz = parseInt(data[0]);
              var konum = parseInt(data[3]);

              if (pozisyon >= 70 && yavaslama >= 8 && hiz >= 70 &&  konum <= 1000) {
                brakeButton.classList.add("active");
              } else {
                brakeButton.classList.remove("active");
              }
            }
          };
          xhr.send();
        }, 1000);
      </script>
    </body>
  </html>
)=====";

void handleRoot() {
  String page = htmlPage;
  page.replace("%HIZ%", String(hiz));
  page.replace("%YAVASLAMA%", String(yavaslama));
  page.replace("%POZISYON%", String(pozisyon));
  page.replace("%KONUM%", String(konum));
  server.send(200, "text/html", page);
}

void handleData() {
  String data = String(hiz) + "," + String(yavaslama) + "," + String(pozisyon) + "," + String(konum);
  server.send(200, "text/plain", data);
}

void setup() {
  Serial.begin(115200);

  // Sabit IP ile WiFi'ye bağlan
  if (!WiFi.config(local_IP, gateway, subnet, primaryDNS, secondaryDNS)) {
    Serial.println("Statik IP yapılandırılamadı.");
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("WiFi'ye bağlanılıyor...");
  }

  Serial.println("WiFi'ye bağlanıldı.");
  Serial.print("Alıcı cihazın sabit IP'si: ");
  Serial.println(WiFi.localIP());

  udp.begin(udpPort);

  // Web server ayarları
  server.on("/", handleRoot);
  server.on("/data", handleData);
  server.begin();
}

void loop() {
  // UDP üzerinden gelen veri varsa oku
  int packetSize = udp.parsePacket();
  if (packetSize) {
    int len = udp.read(incomingPacket, 255);
    if (len > 0) {
      incomingPacket[len] = 0;  // Mesajı sonlandırmak için null-terminate
    }

    // Gelen veriyi parçala ve hız, yavaşlama, pozisyon, konum bilgilerini güncelle
    sscanf(incomingPacket, "Hız:%d,Yavaşlama:%d,Pozisyon:%d,Konum:%d", &hiz, &yavaslama, &pozisyon, &konum);
    Serial.printf("Gelen veri -> Hız: %d, Yavaşlama: %d, Pozisyon: %d, Konum: %d\n", hiz, yavaslama, pozisyon, konum);

    // Geri bildirim gönder (ACK)
    udp.beginPacket(vericiIP, ackPort);
    udp.write((const uint8_t*)"ACK", strlen("ACK"));  // Mesajı byte array'e çevirerek gönderiyoruz
    udp.endPacket();
  }

  // Web sunucuyu yönet
  server.handleClient();
}
