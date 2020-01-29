import socket


ip = "127.0.0.1" # '' for any interface
port = 53535
domainname = "ip.example.com"

ip_label_list = domainname.split('.')
#make binary objects to faster compare
ip_label_list = list(map(lambda x: x.encode(), ip_label_list))
ip_label_list.reverse()


def create_flags(hqr=1, hopcode=1, haa=1, htc=0, hrd=0, hra=0, hz=0, hrcode=0):
    # init flags to 2 byte 0000s
    flags = 0x0000
    # set to answer
    flags = flags | (hqr << 15)
    # set the admin answer
    flags = flags | (haa << 10)
    # recursion desired
    flags = flags | (hrd << 8)
    # recursion avail
    flags = flags | (hra << 7)
    # rcode
    flags = flags | hrcode
    # return the flags as bytes
    return flags.to_bytes(2, byteorder="big")



sock = socket.socket(socket.AF_INET, # IP
        socket.SOCK_DGRAM) # udp

sock.bind((ip, port))
print(f"server started on {ip}:{port}...")

while True:
    ret = []
    # fail set to 0, if it changes along the path, give it out ... TODO
    fail = 0
    # rfc 1123 2.5 635 hostname+header+x (TODO)
    data, addr = sock.recvfrom(647)
    #print(f"connect from {addr} for {data}")
    try:
        request = {}
        request['id'] = data[0:2]
        # we send a reply & 128
        request['type'] = data[2] & 128
        # we only handle queries,
        request['opcode'] = data[2] & 120
        request['aa'] = data[2] & 4
        request['tc'] = data[2] & 2
        request['rd'] = data[2] & 1
        request['ra'] = data[3] & 128
        request['z'] = data[3] & 64
        request['ad'] = data[3] & 32
        request['cd'] = data[3] & 16

        request['qdcount'] = int.from_bytes(data[4:6], byteorder="big")
        request['ancount'] = int.from_bytes(data[6:8], byteorder="big")
        request['nscount'] = int.from_bytes(data[8:10], byteorder="big")
        request['arcount'] = int.from_bytes(data[10:12], byteorder="big")

        # iterate over labels indicating the length of next label
        # 3www6google2de0
        # initial pos @data 12 after header
        pos = 12
        labels = []
        # request is terminated with a %00
        while data[pos] != 0 and pos < len(data):
            # make sure the label is not pointint behind the lenght of the data
            if (pos + data[pos] < len(data)-4 ):
                labels.append(data[ pos + 1 : pos + 1 + data[pos]])
                #move pointer x bytes forward, add the pos byte
                pos += data[pos] + 1
            else:
                # set fail error code to format error
                fail = 1
        #print(f"pointer is @{pos}")
        request['length'] = pos + 1 - 12

        request['type'] =  data[12 + request['length'] : 12 + request['length'] +2]


        for i in ip_label_list:
            if labels[-1] == i:
                del labels[-1]
            else:
                # TODO
                # server fail for domain not found
                pass
        #print("labels after deletion: ", labels)

        # check the request type TODO
        # 1 is an A query
        if int.from_bytes(request['type'], byteorder="big") == 1:
            if len(labels) >= 4:
                answer = []
                allnum = True
                try:
                    # iterate over the first four elements, if all are numbers, below 255, eq 0, we can use it
                    for i in labels[0:4]:
                        if i.isdigit():
                            answer.append( int(i.decode()).to_bytes(1, byteorder="big"))
                        else:
                            allnum = False
                except Exception as e:
                    print("error in convert", e)
                    fail=1;
                else:
                    answer = b"".join(answer)
            if (not allnum):
                fail = 1


        request['class'] = data[12 + request['length'] + 2 : 12 + request['length'] +4]

        #print("request", request)

        # copy the id to the answer
        ret.append(data[0:2])
        # add the flags
        ret.append(create_flags(hrcode = fail))
        # questions  2 byte 1
        ret.append(b'\x00\x01')
        # answers 1
        ret.append(b'\x00\x01')
        # authority
        ret.append(b'\x00\x00')
        # additional resource records
        ret.append(b'\x00\x00')
        # copy the request + type + class
        ret.append(data[12: 12+request['length'] +4])
        # reference to the request name, 2 highest bits 11 pos 12 in req
        ret.append(b'\xc0\x0c')
        # the type
        ret.append(request['type'])
        # the class
        ret.append(request['class'])
        # ttl 32s
        ret.append(b'\x00\x00\x00\x20')
        # data length / 4 bytes
        ret.append(len(answer).to_bytes(2, byteorder="big"))
        # the answer --- finally :-)
        ret.append(answer)
        # additional record
        # ret.append(b'\x00\x00\x29')
        # # udp payload
        # ret.append(b'\x02\x00')
        # # higher bits in rcode
        # ret.append(b'\x00')
        # # edns
        # ret.append(b'\x00')
        # # z
        # ret.append(b'\x00\x00')
        # # data length
        # ret.append(b'\x00\x00')


        #print("return", ret)


        sock.sendto(b"".join(ret), addr)
        print(".", end="", sep="")

    except Exception as e:
        print("error in data:", e)
