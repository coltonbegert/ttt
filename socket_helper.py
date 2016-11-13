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
    assert N > 0
    chunks = []
    totalrecv = 0
    while totalrecv < N:
        chunk = sock.recv(min(N - totalrecv, 2048))
        if chunk == b'' or not chunk:
            raise RuntimeError("Socket input stream broken")
        chunks.append(chunk)
        totalrecv += len(chunk)
    return b''.join(chunks)

def recv_int(sock):
    return recv(sock, 1)[0]
