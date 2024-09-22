#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>

#pragma comment(lib, "Ws2_32.lib")

#define LEADER_IP "192.168.1.103"  // Lider aracýn IP'si
#define LEADER_PORT 9090           // Lider aracýn portu
#define FOLLOWER_PORT 8080         // Takipçi aracýn portu

void sendPosition(float follower_x, float follower_y) {
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
    servaddr.sin_port = htons(LEADER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(LEADER_IP);

    char position[100];
    sprintf(position, "Position X: %.2f, Y: %.2f", follower_x, follower_y);

    sendto(sockfd, position, strlen(position), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    std::cout << "Konum bilgisi gönderildi: " << position << std::endl;

    closesocket(sockfd);
    WSACleanup();
}

void sendFeedbackToLeader(const char* message) {
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
    servaddr.sin_port = htons(LEADER_PORT);
    servaddr.sin_addr.s_addr = inet_addr(LEADER_IP);

    sendto(sockfd, message, strlen(message), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    std::cout << "Lider araca geri bildirim gönderildi: " << message << std::endl;

    closesocket(sockfd);
    WSACleanup();
}

void receiveEmergencyBrakeMessage() {
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
    servaddr.sin_port = htons(FOLLOWER_PORT);

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) == SOCKET_ERROR) {
        std::cerr << "Bind baþarýsýz. Hata kodu: " << WSAGetLastError() << std::endl;
        closesocket(sockfd);
        WSACleanup();
        return;
    }

    len = sizeof(cliaddr);
    std::cout << "Acil fren mesajý bekleniyor..." << std::endl;

    n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
    buffer[n] = '\0';

    std::cout << "Acil fren mesajý alýndý: " << buffer << std::endl;

    // Lider araca geri bildirim gönder
    sendFeedbackToLeader("Takipçi geri bildirimi alýndý");
    
    closesocket(sockfd);
    WSACleanup();
}

int main() {
    float follower_x, follower_y;

    std::cout << "Takipçi aracýn X pozisyonunu girin: ";
    std::cin >> follower_x;
    std::cout << "Takipçi aracýn Y pozisyonunu girin: ";
    std::cin >> follower_y;

    // Konum bilgisini gönder
    sendPosition(follower_x, follower_y);

    // Acil fren mesajýný al
    receiveEmergencyBrakeMessage();

    return 0;
}

