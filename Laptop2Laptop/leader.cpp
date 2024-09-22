#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <cmath>

#pragma comment(lib, "Ws2_32.lib")

#define FOLLOWER_IP "192.168.1.107"  // Takip�i arac�n IP'si
#define FOLLOWER_PORT 8080           // Takip�i arac�n portu
#define LEADER_PORT 9090             // Lider arac�n dinledi�i port

float receiveFollowerPositionAndCalculateDistance(float leader_x, float leader_y) {
    WSADATA wsadata;
    SOCKET sockfd;
    struct sockaddr_in servaddr, cliaddr;
    char buffer[1024];
    int len, n;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        std::cerr << "Winsock ba�lat�lamad�." << std::endl;
        return -1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Soket olu�turulamad�. Hata kodu: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(LEADER_PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        std::cerr << "Bind ba�ar�s�z. Hata kodu: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return -1;
    }

    len = sizeof(cliaddr);
    std::cout << "Takip�iden konum bilgisi bekleniyor..." << std::endl;

    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
    buffer[n] = '\0';

    std::cout << "Takip�iden konum bilgisi al�nd�: " << buffer << std::endl;

    float follower_x, follower_y;
    sscanf(buffer, "Position X: %f, Y: %f", &follower_x, &follower_y);

    float distance = sqrt(pow(leader_x - follower_x, 2) + pow(leader_y - follower_y, 2));
    std::cout << "�ki ara� aras�ndaki mesafe: " << distance << " birim" << std::endl;
    
    closesocket(sockfd);
    WSACleanup();

    return distance;
}

void sendEmergencyBrakeMessage(bool emergency) {
    WSADATA wsadata;
    SOCKET sockfd;
    struct sockaddr_in servaddr;

    if (WSAStartup(MAKEWORD(2, 2), &wsadata) != 0) {
        std::cerr << "Winsock ba�lat�lamad�." << std::endl;
        return;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Soket olu�turulamad�. Hata kodu: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(FOLLOWER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(FOLLOWER_IP);

    const char* message = emergency ? "Emergency Brake Activated!" : "No Emergency Brake";
    sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    std::cout << "Acil fren mesaj� g�nderildi: " << message << std::endl;

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
        std::cerr << "Winsock ba�lat�lamad�." << std::endl;
        return;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd == INVALID_SOCKET) {
        std::cerr << "Soket olu�turulamad�. Hata kodu: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return;
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(LEADER_PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        std::cerr << "Bind ba�ar�s�z. Hata kodu: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    len = sizeof(cliaddr);
    std::cout << "Geri bildirim bekleniyor..." << std::endl;

    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
    buffer[n] = '\0';  

    std::cout << "Takip�iden geri bildirim al�nd�: " << buffer << std::endl;

    closesocket(sockfd);
    WSACleanup();
}

int main() {
    int leader_x, leader_y;
    int speed, deceleration;
    int brake_position;

    std::cout << "Lider arac�n X pozisyonunu girin: ";
    std::cin >> leader_x;
    std::cout << "Lider arac�n Y pozisyonunu girin: ";
    std::cin >> leader_y;

    // Takip�iden konum bilgisi al ve mesafeyi hesapla
    float distance = receiveFollowerPositionAndCalculateDistance(leader_x, leader_y);

    std::cout << "Ara� h�z� (km/h) girin: ";
    std::cin >> speed;
    std::cout << "Yava�lama (m/s^2) girin: ";
    std::cin >> deceleration;
    std::cout << "Fren pedal pozisyonu (%) girin: ";
    std::cin >> brake_position;

    // Acil fren durumunu kontrol et, mesafe 1000 metreden k���kse acil fren mesaj� g�nder
    bool emergency = (speed >= 70 && deceleration > 8 && brake_position > 70 && distance < 1000);
    sendEmergencyBrakeMessage(emergency);
    
    receiveFollowerFeedback();
    
    std::cin.ignore();
    std::cout << "Program� kapatmak i�in bir tu�a bas�n..." << std::endl;
	std::cin.get();  


    return 0;
}



