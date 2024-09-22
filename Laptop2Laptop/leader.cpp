#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cmath>

#pragma comment(lib, "Ws2_32.lib")

#define FOLLOWER_IP "192.168.1.107"  // Takipçi aracýn IP'si
#define FOLLOWER_PORT 8080           // Takipçi aracýn portu
#define LEADER_PORT 9090             // Lider aracýn dinlediði port

float receiveFollowerPositionAndCalculateDistance(float leader_x, float leader_y) {
    WSADATA wsadata;
    SOCKET sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    int len, n;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        std::cerr << "Winsock baþlatýlamadý." << std::endl;
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Soket oluþturulamadý. Hata kodu: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(LEADER_PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        std::cerr << "Bind baþarýsýz. Hata kodu: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    len = sizeof(cliaddr);
    std::cout << "Takipçiden konum bilgisi bekleniyor..." << std::endl;

    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
    buffer[n] = '\0';

    std::cout << "Takipçiden konum bilgisi alýndý: " << buffer << std::endl;

    float follower_x, follower_y;
    sscanf(buffer, "Position X: %f, Y: %f", &follower_x, &follower_y);

    float distance = sqrt(pow(leader_x - follower_x, 2) + pow(leader_y - follower_y, 2));
    std::cout << "Ýki araç arasýndaki mesafe: " << distance << " birim" << std::endl;
    
    closesocket(sockfd);
    WSACleanup();

    return distance;
}

void sendEmergencyBrakeMessage(bool emergency) {
    WSADATA wsadata;
    SOCKET sockfd;
    struct sockaddr_in servaddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        std::cerr << "Winsock baþlatýlamadý." << std::endl;
        return;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Soket oluþturulamadý. Hata kodu: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(FOLLOWER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(FOLLOWER_IP);

    const char* message = emergency ? "Emergency Brake Activated!" : "No Emergency Brake";
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    std::cout << "Acil fren mesajý gönderildi: " << message << std::endl;

    closesocket(sockfd);
    WSACleanup();
}

void receiveFollowerFeedback() {
    WSADATA wsadata;
    SOCKET sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    int len, n;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        std::cerr << "Winsock baþlatýlamadý." << std::endl;
        return;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Soket oluþturulamadý. Hata kodu: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(LEADER_PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        std::cerr << "Bind baþarýsýz. Hata kodu: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    len = sizeof(cliaddr);
    std::cout << "Geri bildirim bekleniyor..." << std::endl;

    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
    buffer[n] = '\0';  

    std::cout << "Takipçiden geri bildirim alýndý: " << buffer << std::endl;

    closesocket(sockfd);
    WSACleanup();
}

int main() {
    int leader_x, leader_y;
    int speed, deceleration;
    int brake_position;

    std::cout << "Lider aracýn X pozisyonunu girin: ";
    std::cin >> leader_x;
    std::cout << "Lider aracýn Y pozisyonunu girin: ";
    std::cin >> leader_y;

    // Takipçiden konum bilgisi al ve mesafeyi hesapla
    float distance = receiveFollowerPositionAndCalculateDistance(leader_x, leader_y);

    std::cout << "Araç hýzý (km/h) girin: ";
    std::cin >> speed;
    std::cout << "Yavaþlama (m/s^2) girin: ";
    std::cin >> deceleration;
    std::cout << "Fren pedal pozisyonu (%) girin: ";
    std::cin >> brake_position;

    // Acil fren durumunu kontrol et, mesafe 1000 metreden küçükse acil fren mesajý gönder
    bool emergency = (speed >= 70 && deceleration > 8 && brake_position > 70 && distance < 1000);
    sendEmergencyBrakeMessage(emergency);
    
    receiveFollowerFeedback();
    
    std::cin.ignore();
    std::cout << "Programý kapatmak için bir tuþa basýn..." << std::endl;
	std::cin.get();  


    return 0;
}



