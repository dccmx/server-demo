#!/usr/bin/python
# coding: utf-8
import socket
import thread
import struct


def nrecv(sock, size):
    data = ''
    while len(data) < size:
        data += sock.recv(size - len(data))
    return data


def handler(clientsock, addr):
    clientsock.send('xxxx')
    for i in range(5):
        data = nrecv(clientsock, 8)
        if not data:
            break
        a, b = struct.unpack('ii', data)
        c = a + b
        data = struct.pack('i', c)
        clientsock.send(data)
    clientsock.close()


def main():
    ADDR = ('0.0.0.0', 1234)
    serversock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    serversock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    serversock.bind(ADDR)
    serversock.listen(5)
    print 'listenning on port 1234...'
    while 1:
        clientsock, addr = serversock.accept()
        thread.start_new_thread(handler, (clientsock, addr))

if __name__ == '__main__':
    main()
