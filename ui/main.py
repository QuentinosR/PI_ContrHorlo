import sys
from PyQt6.QtWidgets import QApplication, QMainWindow
import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QHBoxLayout, QPushButton
from PyQt5.QtGui import QPalette, QColor
import serial

class Flasher():
    def __init__(self, uart_dev, baudrate):
        self.ser = serial.Serial(uart_dev, baudrate)

    def _send_cmd(self, cmd):
        for c in cmd:
            self.ser.write(c.encode())

        self.ser.write(";".encode())

        s = self.ser.read(120)
        return s.decode()
    def on(self):
        self._send_cmd("trig.en:1;")
    def off(self):
        self._send_cmd("trig.en:0;")

flasher = Flasher('/dev/ttyACM0', 115200)

class Color(QWidget):

    def __init__(self, color):
        super(Color, self).__init__()
        self.setAutoFillBackground(True)

        palette = self.palette()
        palette.setColor(QPalette.Window, QColor(color))
        self.setPalette(palette)

class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()

        self.setWindowTitle("UI Flasher")
        self.setGeometry(100, 100, 280, 80)

        layout = QHBoxLayout()

        self.button = QPushButton("On", self)
        self.button.setGeometry(200, 150, 100, 40)
        self.button.setCheckable(True)
        self.button.clicked.connect(self.on_button_clicked)
        
        layout.addWidget(self.button)
        layout.addWidget(Color('red'))
        layout.addWidget(Color('green'))
        layout.addWidget(Color('blue'))


        widget = QWidget()
        widget.setLayout(layout)
        self.setCentralWidget(widget)
    
    def on_button_clicked(self):
        if self.button.isChecked():
            flasher.on()
            self.button.setText("Off")
        else:
            flasher.off()
            self.button.setText("On")

if __name__ == "__main__": 
    app = QApplication(sys.argv)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())
