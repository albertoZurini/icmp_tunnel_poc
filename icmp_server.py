import socket, struct

def listen():
  s = socket.socket(socket.AF_INET,socket.SOCK_RAW,socket.IPPROTO_ICMP)
  s.setsockopt(socket.SOL_IP, socket.IP_HDRINCL, 1)
  while 1:
    data, addr = s.recvfrom(1508)
    print("Packet from %r: %r" % (addr,data))
    icmp_header = data[20:28]
    type, code, checksum, p_id, sequence = struct.unpack('bbHHh', icmp_header)
    print("type: [" + str(type) + "] code: [" + str(code) + "] checksum: [" + str(checksum) + "] p_id: [" + str(p_id) + "] sequence: [" + str(sequence) + "]")

listen()