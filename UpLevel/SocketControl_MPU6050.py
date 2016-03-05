# SocketControl_MPU6050.py
import time
import socket
import serial
from Calibrate import CarAdmin
from threading import Thread
DEFAULT_SPEED = 15


def recv_all(sock, length):
    data = ""
    while len(data) < length:
        more = sock.recv(length - len(data))
        if not more:
            raise EOFError('recv_all')
        data += more
    return data


class CarSocketAdmin(CarAdmin):

    def __init__(self, name, ServerIP, ServerPort, ID):
        CarAdmin.__init__(self, name)
        self.ServerIP = ServerIP
        self.ServerPort = ServerPort
        self.ID = ID
        self.GlobalMem = 0
        self.GlobalFlag = 0
        self.RightAckFlag = 0
        # Port_name_MPU6050 = raw_input("Choose the MPU6050 port\n")
        self.angle = 0
        self.mpu6050_start_angle = 0
        self.turning_angle = 0
        self.Rcv_Buffer = 0
        self.RcvByte = 0
        self.Order_Sock_MPU6050 = 0
        self.angle_over_360 = 0
        self.stop_angle_over_360 = 0
        self.stop_angle_range = [[0, 0], [0, 0]]
        self.first_start = 1

    def TouchTheCar(self):
# if time.time() - self.LastAckTime > 1:
# print "Lost Connect\n"
        if time.time() - self.LastAckTime > 0.5:
            self.SendOrder(self.LastSentOrder)

#     def ReadTheSerial(self):
#         print "ReadTheSerial start"
#         while True:
#             self.RcvBuffer.append(self.port.read(1))
#             if len(self.RcvBuffer) >= 4:
#                 if ord(self.RcvBuffer[0]) == 0x54:
#                     if ord(self.RcvBuffer[1]) + ord(self.RcvBuffer[2]) \
#                             == ord(self.RcvBuffer[3]):
#                         if self.RcvBuffer[1] != self.LastSentOrder:
#                             print "Recv:", self.RcvBuffer[1], "old :", self.LastSentOrder
#                             print "Recv Wrong Acknowledge,Resenting..."
#                             self.SendOrder(self.LastSentOrder)
#                         else:
# print "right ack\n"
#                             self.RightAckFlag = 1
#                             self.LastAckTime = time.time()
#                     else:
#                         print "Damaged message\n"
#                     print "delete 4"
#                     del self.RcvBuffer[0:5]
#                 else:
#                     del self.RcvBuffer[0]

    def SocketClient(self):
        LegalOrder = ['g', 'l', 'r', 'f', 's', 'b']
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        s.bind((SERVERIP, SERVERPORT))
        s.listen(1)
        while True:
            print 'Listening at', s.getsockname()
            sc, sockname = s.accept()
            print 'We have accepted a connection from ', sockname
            print 'Socket connects', sc.getsockname(), 'and', sc.getpeername()
            while True:
                try:
                    message = sc.recv(1024)
                    sc.sendall("ok\n")
                    print "recv", repr(message), '\n'
                    command_tokens = message.split('\n')
                    if command_tokens[0] in ['g', 'f', 's', 'b']:
                        self.Send_Direct_Order(order = command_tokens[0])
                        self.state = command_tokens[0]
                        # self.GlobalMem = command_tokens[0]
                        # self.GlobalFlag = 1
                        # self.Order_Sock_MPU6050 = command_tokens[0]
                    elif command_tokens[0] in ['l', 'r'] and len(command_tokens) >= 2:
                        angle = int(command_tokens[1])
                        self.Send_Direct_Order(order= command_tokens[0], data1=angle, data2 = angle)
                        # self.Order_Sock_MPU6050 = command_tokens[0]
                        # self.turning_angle = int(command_tokens[1])
                except Exception, e:
                    print "*** connection failed.", e, "\n Delete the couple ***"
                    sc.shutdown(socket.SHUT_RDWR)
                    sc.close()
                    break


    def Run(self):
        ThreadSocket = Thread(target=self.SocketClient, args=())
        # ThreadSerialRead = Thread(target=self.ReadTheSerial, args=())
        # ThreadSerialRead.start()
        # ThreadMPU6050 = Thread(target=self.ReadMPU6050, args=())
        ThreadSocket.start()
        # ThreadMPU6050.start()
        while True:
            self.check_last_send()
            self.rcv_uart_msg()
            # self.TouchTheCar()

    def send_order(self, order):
        # if order in ['b', 'l', 'r', 'f'] and self.first_start:
        #     self.first_start = 0
        #     self.Send_Direct_Order(order='go', pwm=DEFAULT_SPEED)
        #     time.sleep(0.05)
        if order == 's':
            self.Send_Direct_Order(order='s')
        elif order == 'l':
            self.Send_Direct_Order(order='l')
        elif order == 'r':
            self.Send_Direct_Order(order='r')
        elif order == 'f':
            self.Send_Direct_Order(order='a')
        elif order == 'b':
            self.Send_Direct_Order(order='b')
if __name__ == '__main__':
    SERVERIP = '127.0.0.1'
    SERVERPORT = 8888
    Admin = CarSocketAdmin('CarCar', SERVERIP, SERVERPORT, 1)
    Admin.Run()
