import socket
import logging


def TCP_Config():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    host = '0.0.0.0'
    port = 12345


    server_socket.bind((host, port))


    server_socket.listen(5)
    print(f"Listening on {host}:{port}...")
    return server_socket


def Tcp_Listening(server_socket):
    while True:
            
        client_socket, client_address = server_socket.accept()
        print(f"Connection from {client_address} has been established!")
        try:
            data = client_socket.recv(1024)
            print(f"Received from client: {data.decode()}")
            buffers = data.decode('utf-8').split()

            print("split data: ")
            print(buffers[0])
            print(buffers[1])

            client_socket.close()
        except Exception as e:
            print(f"Error: {e}")

        finally:
            client_socket.close()






############################################  begin ######################################



server_socket = TCP_Config()
Tcp_Listening(server_socket)