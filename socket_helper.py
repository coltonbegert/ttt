def pack(*args):
    return bytes(args)

def send(sock, packet):
    totalsent = 0
    while totalsent < len(packet):
        sent = sock.send(packet[totalsent:])
        if sent == 0:
            raise RuntimeError("Socket output stream broken")
        totalsent += sent

def recv(sock, N):
    chunks = []
    totalrecv = 0
    while totalrecv < N:
        chunk = sock.recv(min(N - totalrecv, 2048))
        if chunk == '':
            raise RuntimeError("Socket input stream broken")
        chunks.append(chunk)
        totalrecv += len(chunk)
    return ''.join(chunks)
