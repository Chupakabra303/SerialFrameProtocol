from threading import Thread
import serial
import time
import random
import struct
from queue import Queue, Empty
import os
from tkinter import *
from tkinter.messagebox import *
from tkinter.scrolledtext import *

class SerialFrameProtocol:
    # width=16 poly=0x1021 init=0xffff refin=false refout=false xorout=0x0000 check=0x29b1 residue=0x0000 name="CRC-16/IBM-3740"
    def CRC16(self, data: bytes):
        crc = 0xFFFF
        for b in data:
            crc ^= b << 8
            for i in range(8):
                if (crc & 0x8000):
                    crc = (crc << 1) & 0xFFFF ^ 0x1021
                else:
                    crc = (crc << 1) & 0xFFFF
        return crc

    def LRC(self, data: bytes):
        return -sum(data) & 0xFF

    def __init__(self, serialPort, checkLen=2):
        self.serialPort = serialPort  # класс последовательного порта
        self.checkLen = checkLen  # checkLen=1 - LRC, checkLen=2 - CRC16
        if checkLen == 2:
            self.checkFun = self.CRC16
        else:
            self.checkFun = self.LRC

    def frameEncoder(self, data: bytes):
        return (data + self.checkFun(data).to_bytes(self.checkLen, 'big')).replace( b'\\', b'\\e').replace(b'\n', b'\\n') + b'\n'

    def frameDecoder(self, frame: bytes):
        data = frame[:-1].replace(b'\\n', b'\n').replace(b'\\e', b'\\')
        print(data)
        if len(data) < self.checkLen + 1:
            raise serial.SerialException("Serial Frame Protocol. Frame length error")
        elif self.checkFun(data) != 0:
            raise serial.SerialException("Serial Frame Protocol. Error CRC")
        else:
            print('len = ', len(data))
            if self.checkLen == 2:
                print('CRC16 = ', hex(self.checkFun(data)))
            else:
                print('LRC = ', hex(self.checkFun(data)))
            data = data[:-self.checkLen]
        return data

    def write(self, data):
        self.serialPort.write(self.frameEncoder(data))

    def read(self):
        if self.serialPort.in_waiting > 0:
            frame = com.read_until(b'\n')
            return self.frameDecoder(frame)
        else:
            return b''

def sendArray(num):
    listX = [random.getrandbits(8) for _ in range(num)]
    sfp.write(b'CX' + bytes(listX))
    queueLogMsg.put("Отправлен массив AX. Длина: {}. Содержимое: {}".format(num, ','.join(map(str, listX))))

def SerialReadDaemon():
    queueLogMsg.put("SerialRead daemon запущен")
    while not stop_threads:
        try:
            data = sfp.read()
            # print("SerialReadDeamon run")
            if len(data) > 0:
                ansPrefix = data[:2];
                if ansPrefix == b'B0':
                    values = struct.unpack('<if', data[2:])
                    queueLogMsg.put(data)
                    queueLogMsg.put("Получен блок B0: iVal={0[0]}; fVal={0[1]}".format(values))
                elif ansPrefix == b'A0': # массив int32_t  с размером uint32_t в начале
                    arraySize = struct.unpack('<I', data[2:6])[0]
                    values = struct.unpack('<{}i'.format(arraySize), data[6:])
                    queueLogMsg.put("Получен массив A0. Длина: {}. Содержимое: {}".format(arraySize, ','.join(map(str, values))))
                elif ansPrefix == b'A1': # массив uint16_t с размером uint32_t в начале
                    arraySize = struct.unpack('<I', data[2:6])[0]
                    values = struct.unpack('<{}H'.format(arraySize), data[6:])
                    queueLogMsg.put("Получен массив A1. Длина: {}. Содержимое: {}".format(arraySize, ','.join(map(str, values))))
                elif ansPrefix == b'AX':  # массив uint8_t без размера в начале
                    arraySize = len(data[2:])
                    values = struct.unpack('<{}B'.format(arraySize), data[2:])
                    queueLogMsg.put("Получен массив AX. Длина: {}. Содержимое: {}".format(arraySize, ','.join(map(str, values))))
                else:
                    queueLogMsg.put(data.decode('utf-8')) # преобразуем в utf-8 и отображаем в журнале
            else:
                # queueLogMsg.put("No read data")
                time.sleep(0.2)
            # queueLogMsg.put(ser.read(ser.in_waiting))
        except serial.SerialException as e:
            # нет sleep из-за таймаута открытия соединения
            print(e)
            queueLogMsg.put(e)
        except Exception as e:
            print(e)
            queueLogMsg.put(e)
            time.sleep(1)

def writeToLog(msg):
    # https://tkdocs.com/tutorial/text.html#basics
    numlines = int(log.index('end - 1 line').split('.')[0])
    log['state'] = NORMAL
    if numlines > 100:
        log.delete(1.0, 'end - 100 line')
    # if log.index('end - 1 char')!='1.0':
    # log.insert('end', '\n')
    log.insert(END, msg)
    log.insert(END, '\n')
    log.see('end - 1 line')
    log['state'] = DISABLED

def textClear():
    log['state'] = NORMAL
    log.delete("1.0", END)
    log['state'] = DISABLED

def on_closing():
    if askokcancel("Выход", "Завершить работу?"):
        window.destroy()

def windowTimer1(interval):
    try:
        while True:
            writeToLog(queueLogMsg.get_nowait())
    except Empty:
        pass
    window.after(interval, windowTimer1, interval)
# ------------------------

# print(hex(LRC(b'12345')))
queueLogMsg = Queue()

com = serial.Serial('COM20', baudrate=57600, timeout=1)
sfp = SerialFrameProtocol(com, checkLen=2)  # checkLen=1 - LRC, checkLen=2 - CRC16

window = Tk()
window.title(os.path.basename(__file__))
window.protocol("WM_DELETE_WINDOW", on_closing)
window.geometry("{}x{}".format(int(window.winfo_screenwidth()*0.8), int(window.winfo_screenheight()*0.8)))
# window.geometry("{}x{}".format(window.winfo_screenwidth()//3, window.winfo_screenheight()//3))

topFrame = Frame(window, borderwidth=2)
bottomFrame = Frame(window)
topFrame.pack(pady=0, padx=0, expand=0, fill=BOTH)
bottomFrame.pack(pady=0, padx=0, expand=1, fill=BOTH)

Button(topFrame, text="CN", width=15, height=2, command=lambda: sfp.write(b'CN'))\
    .grid(row=0, column=0, pady=2, padx=2)
Button(topFrame, text="CY", width=15, height=2, command=lambda: sfp.write(b'CY'))\
    .grid(row=0, column=1)
Button(topFrame, text="CZ", width=15, height=2, command=lambda: sfp.write(b'CZ'))\
    .grid(row=0, column=2)
Button(topFrame, text="CL", width=15, height=2, command=lambda: sfp.write(b'CL'))\
    .grid(row=0, column=3)
Button(topFrame, text="CB\n(get iVal, fVal) ", width=15, height=2, command=lambda: sfp.write(b'CB'))\
    .grid(row=0, column=4)
Button(topFrame, text="CA\n(get arrays)", width=15, height=2, command=lambda: sfp.write(b'CA'))\
    .grid(row=1, column=0)
Button(topFrame, text="CX\n(send/get arrays)", width=15, height=2, command=lambda: sendArray(100))\
    .grid(row=1, column=1)

# clearButton.pack(side=BOTTOM, pady=5, padx=2)
# errorLabel = Label(topFrame, text="12.231", width=12, height=2, bg='white')
# errorLabel.grid(row=1, column=1)

Button(topFrame, text="Очистить вывод", width=15, height=2, command=textClear).grid(row=1, column=4)

log = ScrolledText(bottomFrame, height=7)
log.pack(pady=2, padx=2, expand=1, fill=BOTH)

window.update()

windowTimer1(100)  # запуск 1000мс цикла функций в потоке графики

stop_threads = False
t1 = Thread(target=SerialReadDaemon, daemon=True)
t1.start()
window.mainloop()
stop_threads = True
t1.join()
print("Работа завершена")
