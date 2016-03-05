# Calibrate.py
'''
This software is for Tang's car.
'''
import serial
import Tkinter
import time
import pickle
import pdb


class CarAdmin():

    def __init__(self, name):
        self.name = name
        self.State = 's'
        self.RcvBuffer = []
        self.LastSentOrder = 's'
        self.LastAckTime = 0
        print("serial port available:\n")
        print(self.serial_ports())
        sPortChoic = raw_input("Input the port to open\n")
        self.port = serial.Serial(sPortChoic, 9600)
        self.SentOrderRecord = 0
        self.last_direct_order = 0
        self.LeftRotate = 0
        self.RightRotate = 0
        self.calibra_state = 0
        self.last_order_time = 0
        self.pwm_degree = 1
        self.MAX_PWM_DEGREE = 4
        self.pwm_preset = []
        self.debug_left_pwm_buf = 0
        self.debug_right_rotate_buf = 0
        self.debug_left_rotate_buf = 0
        self.debug_right_rotate_buf = 0
        self.debug_left_pwm_buf = 0
        self.debug_right_pwm_buf = 0
        self.cycle_pair = {'f': 'b', 'b': 'f'}
        self.cycle_state = 0
        self.last_cycle_time = 0
        self.state = 0
        self.last_order = 0
        self.send_msg_time = 0

    def TurnLeft(self):
        self.State = 'l'
        self.Send_Direct_Order(order='l', data1=90, data2 = 90)

    def TurnRight(self):
        self.state = 'r'
        self.Send_Direct_Order(order='r', data1=90, data2 = 90)

    def Forward(self):
        self.state = 'f'
        self.Send_Direct_Order(order='f')

    def Backward(self):
        self.state = 'b'
        self.Send_Direct_Order(order='b')

    def Stop(self):
        self.state = 's'
        self.Send_Direct_Order(order='s')

    def cycle(self):
        self.state = 'cycle'
        self.Send_Direct_Order(order='f')
        self.last_cycle_time = time.time()
        self.cycle_state = 'f'

    def update_pwm_A(self):
        self.pwm = int(self.pwm_entry.get())
        print "Your pwm:%s" % self.pwm_entry.get()
        self.Send_Direct_Order(order='A', data1=self.pwm/256,
                               data2=self.pwm % 256)

    def update_pwm_B(self):
        self.pwm = int(self.pwm_entry.get())
        print "Your pwm:%s" % self.pwm_entry.get()
        self.Send_Direct_Order(order='B', data1=self.pwm/256,
                               data2=self.pwm % 256)

    def Send_Direct_Order(self, PWM_left=None, PWM_right=None, order=None,
                          data1=None, data2=None):
        if(order is None):
            msg = '$DCR:' + str(PWM_left) + str(-500) + \
                ',' + str(PWM_right) + str(-500) + '!'
            self.port.write(msg)
            print msg, '\n'
        else:
            if data1 is None or data2 is None:
                data1 = 0x00
                data2 = 0x00
            self.port.write(['H'])
            self.port.write([order])
            self.port.write([data1])
            self.port.write([data2])
            if order in ['l', 'r']:
                self.last_order = 'k'
            else:
                self.last_order = order
            self.last_data1 = data1
            self.last_data2 = data2
            self.send_msg_time = time.time()
        return 0

    def serial_ports(self):
        """Lists serial ports

        :raises EnvironmentError:
            On unsupported or unknown platforms
        :returns:
            A list of available serial ports
        """
        ports = ['COM' + str(i + 1) for i in range(256)]
        result = []
        for port in ports:
            try:
                s = serial.Serial(port)
                s.close()
                result.append(port)
            except (OSError, serial.SerialException):
                pass
        return result

    def check_last_send(self):
        if time.time() - self.send_msg_time > 0.5 and self.last_order != 0:
            self.Send_Direct_Order(order=self.last_order,
                                   data1=self.last_data1,
                                   data2=self.last_data2)
            self.send_msg_time = time.time()
            # print "resent"+self.last_order +'\n'

    def rcv_uart_msg(self):
        byte_2_read = self.port.inWaiting()
        if byte_2_read >= 4:
            rcv = self.port.read(byte_2_read)
            # if 'l_ok' in rcv or 'r_ok' in rcv:
            #     self.last_order = 'k'
            # if 'ok' in rcv:
            #     self.last_order = 0
            # print 'recv:', rcv

    def Run(self):
        self.calibra_panel = Tkinter.Tk()
        self.Forward_Button = Tkinter.Button(
            self.calibra_panel, text="Forward",
            command=self.Forward)
        self.Forward_Button.pack()

        self.Backward_button = Tkinter.Button(
            self.calibra_panel, text="Backward",
            command=self.Backward)
        self.Backward_button.pack()

        self.Left_button = Tkinter.Button(
            self.calibra_panel, text="Left",
            command=self.TurnLeft)
        self.Left_button.pack()

        self.Right_button = Tkinter.Button(
            self.calibra_panel, text="Right",
            command=self.TurnRight)
        self.Right_button.pack()

        self.Stop_button = Tkinter.Button(
            self.calibra_panel, text="stop", command=self.Stop)
        self.Stop_button.pack()

        self.Cycle_button = Tkinter.Button(
            self.calibra_panel, text="cycle", command=self.cycle)
        self.Cycle_button.pack()

        self.pwm_set = Tkinter.StringVar()
        self.pwm_entry = Tkinter.Entry(
            self.calibra_panel, textvariable=self.pwm_set)
        self.pwm_entry.pack()

        self.pwm_update_btn = Tkinter.Button(
            self.calibra_panel, text="pwm A change", command=self.update_pwm_A)
        self.pwm_update_btn.pack()

        self.pwm_B_update_btn = Tkinter.Button(
            self.calibra_panel, text="pwm B change", command=self.update_pwm_B)
        self.pwm_B_update_btn.pack()
        while True:
            self.calibra_panel.update()
            self.check_last_send()
            self.rcv_uart_msg()
            if self.state == "cycle":
                if time.time() - self.last_cycle_time >= 4:
                    self.cycle_state = self.cycle_pair[self.cycle_state]
                    self.Send_Direct_Order(order=self.cycle_state)
                    self.last_cycle_time = time.time()

if __name__ == '__main__':

    Admin = CarAdmin('Car')
    Admin.Run()
