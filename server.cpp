#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <cstring>
#include <mutex>
#include <algorithm>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define PORT 54000

std::vector<int> clients; // Lista de clientes conectados
std::mutex clients_mutex; // Controle de acesso à lista

void broadcast(const std::string &message, int sender_sock)
{
  std::lock_guard<std::mutex> lock(clients_mutex);
  for (int client : clients)
  {
    if (client != sender_sock)
    {
      send(client, message.c_str(), message.size() + 1, 0);
    }
  }
}

void handle_client(int client_socket)
{
  char buffer[4096];
  std::string welcome = "Um novo cliente entrou no chat!\n";
  broadcast(welcome, client_socket);

  while (true)
  {
    memset(buffer, 0, 4096);
    int bytes_recv = recv(client_socket, buffer, 4096, 0);
    if (bytes_recv <= 0)
    {
      std::cout << "Cliente desconectado.\n";
      close(client_socket);
      {
        std::lock_guard<std::mutex> lock(clients_mutex);
        clients.erase(std::remove(clients.begin(), clients.end(), client_socket), clients.end());
      }
      std::string leave_msg = "Um cliente saiu do chat.\n";
      broadcast(leave_msg, client_socket);
      break;
    }

    std::string msg = "Cliente " + std::to_string(client_socket) + ": " + buffer;
    std::cout << msg;
    broadcast(msg, client_socket);
  }
}

int main()
{
  int server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket == -1)
  {
    std::cerr << "Erro ao criar socket\n";
    return 1;
  }

  sockaddr_in server_addr{};
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(PORT);
  server_addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(server_socket, (sockaddr *)&server_addr, sizeof(server_addr)) == -1)
  {
    std::cerr << "Erro no bind\n";
    return 1;
  }

  if (listen(server_socket, 5) == -1)
  {
    std::cerr << "Erro no listen\n";
    return 1;
  }

  std::cout << "Servidor rodando na porta " << PORT << "...\n";

  while (true)
  {
    sockaddr_in client_addr{};
    socklen_t client_size = sizeof(client_addr);
    int client_socket = accept(server_socket, (sockaddr *)&client_addr, &client_size);

    if (client_socket == -1)
    {
      std::cerr << "Erro ao aceitar conexão\n";
      continue;
    }

    {
      std::lock_guard<std::mutex> lock(clients_mutex);
      clients.push_back(client_socket);
    }

    std::cout << "Novo cliente conectado: " << client_socket << "\n";
    std::thread(handle_client, client_socket).detach();
  }

  close(server_socket);
  return 0;
}
