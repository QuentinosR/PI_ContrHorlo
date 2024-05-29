import sys
from PyQt6.QtWidgets import QApplication, QMainWindow, QWidget, QHBoxLayout, QVBoxLayout, QPushButton, QSlider, QLabel, QLineEdit
from PyQt6.QtGui import QPalette, QColor
from PyQt6.QtCore import Qt
import serial

class Flasher():
    def __init__(self, uart_dev, baudrate):
        self.ser = serial.Serial(uart_dev, baudrate)

    def _send_cmd(self, cmd):
        for c in cmd:
            self.ser.write(c.encode())

        self.ser.write(";".encode())

        #s = self.ser.read(120)
        #return s.decode()
    def on(self):
        self._send_cmd("trig.en:1;")
    def off(self):
        self._send_cmd("trig.en:0;") 
    def trig_off(self, val):
        self._send_cmd("trig.off:" + str(val) +";")
    def trig_expo(self, val):
        self._send_cmd("trig.expo:" + str(val) +";")
    def trig_shift(self, val):
        self._send_cmd("trig.shift:" + str(val) +";")
    def flash_on(self, val):
        self._send_cmd("flash.on:" + str(val) +";")
    def flash_off(self, val):
        self._send_cmd("flash.off:" + str(val) +";")


flasher = Flasher('/dev/ttyACM0', 115200)

class CommandWidget(QWidget):

    def __init__(self, text, cb_parent_val_changed, min, max, default):
        super(CommandWidget, self).__init__()
        self.text_val = text
        self.parent_cb = cb_parent_val_changed

        self.setMaximumHeight(150)
        
        self.sl = QSlider(Qt.Orientation.Horizontal, self)
        self.sl.setRange(min, max)
        self.sl.setValue(default)
        self.sl.setSingleStep(150)
        self.sl.valueChanged.connect(self.update_val)

        self.text = QLabel("", self)
        self.text.setAlignment(Qt.AlignmentFlag.AlignCenter)
        self.text.setText(self.text_val)

        self.field = QLineEdit()
        self.field.setFixedWidth(75)
        self.field.textChanged.connect(self.update_val)

        self.button = QPushButton('Send')
        self.button.clicked.connect(self.cb_button)
        self.button.setMaximumWidth(50)

        self.update_val(default)

        layout = QVBoxLayout()
        layout.addWidget(self.text)
        layout.addWidget(self.sl)
        layout.addWidget(self.field, alignment= Qt.AlignmentFlag.AlignCenter)
        layout.addWidget(self.button, alignment= Qt.AlignmentFlag.AlignCenter)
        self.setLayout(layout)

    def update_val(self, new_val):
        self.val = int(new_val)
        self.sl.setValue(int(new_val))
        self.field.setText(str(new_val))

    def cb_button(self):
        self.parent_cb(self.val)


class MainWindow(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("UI Flasher")
        self.setGeometry(400, 400, 400, 400)

        layout = QHBoxLayout()

        self.on_button = QPushButton("On", self)
        self.on_button.setGeometry(200, 150, 100, 40)
        self.on_button.setCheckable(True)
        self.on_button.clicked.connect(self.on_button_clicked)

        self.trig_off_widget = CommandWidget('Trig off time (us)', self.trig_off_widget_cb, 1000, 1000000, 20000)
        self.flash_off_widget = CommandWidget('Flash off time (us)', self.flash_off_widget_cb, 1, 100000, 500)
        self.flash_on_widget = CommandWidget('Flash on time (us)', self.flash_on_widget_cb, 1, 100000, 2000)
        self.trig_shift_widget = CommandWidget('Trig shift time (us)', self.trig_shift_cb, 0, 100000, 100)
        self.trig_expo_widget = CommandWidget('Minimal exposure time (us)', self.trig_shift_cb, 0, 200, 19)

        layout.addWidget(self.on_button)
        layout.addWidget(self.trig_off_widget)
        layout.addWidget(self.flash_off_widget)
        layout.addWidget(self.flash_on_widget)
        layout.addWidget(self.trig_shift_widget)
        layout.addWidget(self.trig_expo_widget)

        widget = QWidget()
        widget.setLayout(layout)
        self.setCentralWidget(widget)
    
    def on_button_clicked(self):
        if self.on_button.isChecked():
            flasher.on()
            self.on_button.setText("Off")
        else:
            flasher.off()
            self.on_button.setText("On")
    def trig_off_widget_cb(self, value):
        flasher.trig_off(value)
        print(value)
    def flash_off_widget_cb(self, value):
        flasher.flash_off(value)
        print(value)
    def flash_on_widget_cb(self, value):
        flasher.flash_on(value)
        print(value)
    def trig_shift_cb(self, value):
        flasher.trig_shift(value)
        print(value)
    def trig_expo_cb(self, value):
        flasher.trig_expo(value)
        print(value)
        

if __name__ == "__main__": 
    app = QApplication(sys.argv)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())
