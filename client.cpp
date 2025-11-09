#include <iostream>
#include <thread>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 54000

void receive_messages(int sock) {
    char buffer[4096];
    while (true) {
        memset(buffer, 0, 4096);
        int bytes_recv = recv(sock, buffer, 4096, 0);
        if (bytes_recv <= 0) {
            std::cout << "Desconectado do servidor.\n";
            break;
        }
        std::cout << "\r" << buffer << "\nVocê: " << std::flush;
    }
}

int main() {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        std::cerr << "Erro ao criar socket\n";
        return 1;
    }

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        std::cerr << "Erro ao conectar ao servidor\n";
        return 1;
    }

    std::cout << "Conectado ao chat!\n";

    std::thread(receive_messages, sock).detach();

    while (true) {
        std::string msg;
        std::cout << "Você: ";
        std::getline(std::cin, msg);

        if (msg == "/sair") {
            break;
        }

        int send_res = send(sock, msg.c_str(), msg.size() + 1, 0);
        if (send_res == -1) {
            std::cerr << "Erro ao enviar mensagem\n";
            break;
        }
    }

    close(sock);
    return 0;
}
