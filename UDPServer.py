from socket import *
import os


# create server socket
server_socket = socket(AF_INET, SOCK_DGRAM)
# bind server socket to port number
server_socket.bind(('', 12000))
print("Server is ready to receive messages...\n")

# run forever loop to keep receiving messages
while (True):
    # receive the message from socket
    username, password,client_address = server_socket.recvfrom(2048)
    user = username.decode()
    
    #store username
    filename = ("user.txt")
    userfile = open(filename, "w")
    userfile.write(user)
    userfile.close()
    print("\nMessage sent\n")
