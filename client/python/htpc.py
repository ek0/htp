
import socket
from ctypes import *
import struct


class MessageHeader(Structure):
    _fields_ = [
        ("magic", c_uint),
        ("type", c_uint),
        ("message_size", c_uint)
    ]


class ReadMemoryMessage(Structure):
    _fields_ = [
        ("address", c_uint),
        ("size", c_uint)
    ]

MessageReadMemory = 0
MessageWriteMemory = 1


class HTPClient:
    def __init__(self, host="localhost", port=27023):
        self.clientsocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.clientsocket.connect((host, port))

    def send_message(self, data):
        self.clientsocket.send(data)

    def recv_message(self, data):
        return self.clientsocket.recv(8192)

    def write_memory(self, address, data):
        self.send_message(MessageHeader(0x00505448, MessageWriteMemory, sizeof(c_void_p) + len(data) + sizeof(c_uint)))
        self.send_message(struct.pack("<QQs", address, len(data), data))

    def read_memory(self, address, size):
        self.send_message(MessageHeader(0x00505448, MessageReadMemory, sizeof(c_void_p) + sizeof(c_uint)))
        self.send_message(struct.pack("<QI", address, size))
        return self.clientsocket.recv(size)

    def load_dll(self, path):
        pass

    def close(self):
        pass


if __name__ == "__main__":
    client = HTPClient()
    # TODO: change this ugly fucking shit
    print struct.unpack("<I", client.read_memory(0x00007FF687A2E000, 4))[0]
