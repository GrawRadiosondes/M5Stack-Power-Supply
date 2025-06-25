#
# Copyright (c) 2021  Keep It Simple Solutions
# Modified in 2024 for openWave by TjarkG
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in the
#    documentation and/or other materials provided with the distribution.
# 3. Neither the name of the copyright holders nor contributors may be
#    used to endorse or promote products derived from this software
#    without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT
# HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
#
# Python bindings for liblxi
#
import sys

from ctypes import *
from ctypes.util import find_library
from enum import Enum

# Find library
liblxi = find_library('lxi')
if not liblxi:
    raise FileNotFoundError("Could not find liblxi library")

# Load the library
lib = cdll.LoadLibrary(liblxi)

# Set default timeout
lxi_timeout = 3000


# Define types
class LxiInfo(Structure):
    _fields_ = [("broadcast", CFUNCTYPE(None, c_char_p, c_char_p)),
                ("device", CFUNCTYPE(None, c_char_p, c_char_p)),
                ("service", CFUNCTYPE(None, c_char_p, c_char_p, c_char_p, c_int))]


class Protocol(Enum):
    VXI11 = 0
    RAW = 1


class DiscoverProtocol(Enum):
    VXI11 = 0
    MDNS = 1


# Default functions, overwrite when using device discovery
class LxiInfoClass:
    def __init__(self):
        self.infos = []
    def broadcast(self, address, interface):
        #address = str(address, 'ascii')
        #interface = str(interface, 'ascii')
        #print("Broadcasting on " + interface + " using address " + address)
        pass

    def device(self, address, dev_id):
        address = str(address, 'ascii')
        dev_id = str(dev_id, 'ascii')
        self.infos.append((address, dev_id))
        print("   Found " + dev_id + " on address " + address)

    def service(self, address, dev_id, service, port):
        address = str(address, 'ascii')
        dev_id = str(dev_id, 'ascii')
        service = str(service, 'ascii')
        port = str(port)
        self.infos.append((address, dev_id))
        print("Found " + dev_id + " on address " + address)
        print(" " + service + " service on port " + port)

    def get_ip(self, id_str):
        for info in self.infos:
            if info[1].startswith(id_str):
                return info[0]
        raise RuntimeError(f"No Device with ID String {id_str} found")


def discover(info: LxiInfoClass, protocol: DiscoverProtocol):
    lib.lxi_discover.arg_types = c_void_p, c_int, c_int
    lib.lxi_discover.restype = c_int
    BROADCAST_FUNC = CFUNCTYPE(None, c_char_p, c_char_p)
    broadcast_func = BROADCAST_FUNC(info.broadcast)
    DEVICE_FUNC = CFUNCTYPE(None, c_char_p, c_char_p)
    device_func = DEVICE_FUNC(info.device)
    SERVICE_FUNC = CFUNCTYPE(None, c_char_p, c_char_p, c_char_p, c_int)
    service_func = SERVICE_FUNC(info.service)
    c_info_p = pointer(LxiInfo(broadcast_func, device_func, service_func))
    status = lib.lxi_discover(c_info_p, c_int(lxi_timeout), c_int(protocol.value))
    return status


class Device:
    def __init__(self, address, port: int, name, protocol: Protocol):
        lib.lxi_connect.arg_types = c_char_p, c_int, c_char_p, c_int, c_int
        lib.lxi_connect.restype = c_int
        address_bytes = str.encode(address, "ascii")
        name_bytes = str.encode(name, "ascii")
        self.device = lib.lxi_connect(address_bytes, c_int(port), name_bytes, c_int(lxi_timeout), c_int(protocol.value))
        if self.device < 0:
            raise RuntimeError(f"Device {name} at {address}:{port} not found")

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc_value, traceback):
        lib.lxi_disconnect.arg_types = (c_int,)
        lib.lxi_disconnect.restype = c_int
        status = lib.lxi_disconnect(self.device)
        return status

    # Send LXI Message, return True if successful. If "verbose" is true, print error message before returning False
    def send(self, message: str | bytes, verbose: bool = True) -> bool:
        if type(message) is str:
            message_bytes = message.encode('utf-8')
        else:
            message_bytes = message

        if (len(message_bytes)) > 1500:
            print("Error: sending more than 1500 Byte is currently not possible (liblxi TCP Bug)", file=sys.stderr)
            return False

        lib.lxi_send.arg_types = c_int, c_char_p, c_int, c_int
        lib.lxi_send.restype = c_int
        if lib.lxi_send(c_int(self.device), message_bytes, c_int(len(message_bytes)), c_int(lxi_timeout)) == len(
                message_bytes):
            return True

        if verbose and len(message) > 32:
            print("Error Sending LXI Message " + message[0:32] + "...", file=sys.stderr)
        else:
            print("Error Sending LXI Message", message, file=sys.stderr)
        return False

    # Receive LXI Message of up to length bytes, return message if successful and empty string otherwise.
    # If "verbose" is true, print error message before returning empty string
    def receive(self, length: int = 1024, verbose: bool = True) -> str:
        message_p = create_string_buffer(length)

        lib.lxi_receive.arg_types = c_int, c_char_p, c_int, c_int
        lib.lxi_receive.restype = c_int
        if lib.lxi_receive(c_int(self.device), message_p, c_int(length), c_int(lxi_timeout)) != -1:
            message = str(message_p.value, "ascii")
            return message

        if verbose:
            print("Error Receiving LXI Message", file=sys.stderr)
        return ""

    # Receive LXI Message of up to length bytes, return message if successful and empty string otherwise.
    # If "verbose" is true, print error message before returning empty string
    def receive_raw(self, length: int = 1024, verbose: bool = True) -> bytes:
        message_p = create_string_buffer(length)

        lib.lxi_receive.arg_types = c_int, c_char_p, c_int, c_int
        lib.lxi_receive.restype = c_int
        if lib.lxi_receive(c_int(self.device), message_p, c_int(length), c_int(lxi_timeout)) != -1:
            message = bytes(message_p.value)
            return message

        if verbose:
            print("Error Receiving LXI Message", file=sys.stderr)
        return bytes()
