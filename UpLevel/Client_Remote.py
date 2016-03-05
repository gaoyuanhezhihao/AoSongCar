# Client_Indoor.py
import socket
import pdb

SERVERIP = "127.0.0.1"
SERVERPORT = 8888
s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
while True:
    try:
        s.connect((SERVERIP, SERVERPORT))
        s.sendall("Boss\nBoss1\nConnectCarCar\nCar1")

        while True:
            error_count = 0
            msg = raw_input("Your command:")
            if msg in ['l', 'r']:
                angle = raw_input("angle:")
                msg +='\n'
                msg += angle
            try:
                s.sendall(msg+'\n')
            except Exception, e:
                error_count += 1
                if error_count > 3:
                    print "***ERROR:", e
                    print "connection is down, Rebuild the connection:"
            print "recv:", s.recv(1024)
    except Exception, e:
        print "*** connection failed. \n", e, "\nRetrying"
        pdb.set_trace()
