import sys
from PyQt6.QtWidgets import QApplication, QMainWindow, QWidget, QHBoxLayout, QVBoxLayout, QPushButton, QSlider, QLineEdit, QGroupBox, QTextEdit, QLabel
from PyQt6.QtCore import Qt
import serial
import os
import threading
from PyQt6.QtCore import pyqtSignal
from PyQt6.QtGui import QIcon

class SerialLogger(QWidget):
    sig_recv = pyqtSignal(str)
    def __init__(self, ser):
        super().__init__()
        self.ser = ser
        self.sig_recv.connect(self.display_data)
        threading.Thread(target=self.read_data, daemon=True).start()

        self.initUI()

    def initUI(self):
        widget_layout = QVBoxLayout()
        self.title = QLabel("UART logs")
        self.text_edit = QTextEdit()
        self.text_edit.setReadOnly(True)
        self.text_edit.setMinimumWidth(300)
        widget_layout.addWidget(self.title)
        widget_layout.addWidget(self.text_edit)

        self.setLayout(widget_layout)

    def display_data(self, data):
        #current_text = self.text_edit.toPlainText()
        #new_text = current_text + data
        # Keep only the last 100 characters
        #truncated_text = new_text[-500:]
        #self.text_edit.setPlainText(truncated_text)
        self.text_edit.append(data)
        self.text_edit.ensureCursorVisible()  # Scroll

    def read_data(self):
        while 1:
            data = self.ser.readline().decode('utf-8').strip()
            self.sig_recv.emit(data)

class Flasher():
    def __init__(self, ser):
        self.ser = ser

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


flasher = None

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
        self.sl.setMinimumWidth(400)
        self.sl.valueChanged.connect(self.update_val)

        self.field = QLineEdit()
        self.field.setFixedWidth(75)
        self.field.textChanged.connect(self.update_val)

        self.button = QPushButton('Send')
        self.button.clicked.connect(self.cb_button)
        self.button.setMaximumWidth(50)

        self.gb = QGroupBox(self.text_val)
        self.gb_layout = QVBoxLayout()
        self.gb_layout.addWidget(self.sl)
        self.gb_layout.addWidget(self.field, alignment= Qt.AlignmentFlag.AlignCenter)
        self.gb_layout.addWidget(self.button, alignment= Qt.AlignmentFlag.AlignCenter)
        self.gb.setLayout(self.gb_layout)
        
        widget_layout = QVBoxLayout()
        widget_layout.addWidget(self.gb) 
        self.setLayout(widget_layout)

        self.update_val(default)
        self.cb_button()

    def update_val(self, new_val):
        self.val = int(new_val)
        self.sl.setValue(int(new_val))
        self.field.setText(str(new_val))

    def cb_button(self):
        self.parent_cb(self.val)


class MainWindow(QMainWindow):
    def __init__(self, ser):
        super().__init__()
        self.flasher = Flasher(ser)
        self.serial_logger = SerialLogger(ser)

        self.setWindowTitle("UI Flasher")
        self.setGeometry(600, 600, 600, 600)
        self.setWindowIcon(QIcon('ico.svg'))  # Set window icon

        self.on_button = QPushButton("On", self)
        self.on_button.setGeometry(200, 150, 100, 40)
        self.on_button.setCheckable(True)
        self.on_button.setMaximumWidth(50)
        self.on_button.clicked.connect(self.on_button_clicked)
        self.on_button.setChecked(False)
        self.on_button_clicked()
        #self.on_button.setChecked(False)

        self.trig_off_widget = CommandWidget('Trig off time (us)', self.trig_off_widget_cb, 1000, 1000000, 20000)
        self.flash_off_widget = CommandWidget('Flash off time (us)', self.flash_off_widget_cb, 1, 100000, 500)
        self.flash_on_widget = CommandWidget('Flash on time (us)', self.flash_on_widget_cb, 1, 100000, 2000)
        self.trig_shift_widget = CommandWidget('Trig shift time (us)', self.trig_shift_cb, 0, 100000, 100)
        self.trig_expo_widget = CommandWidget('Minimal exposure time (us)', self.trig_shift_cb, 0, 200, 19)

        layout_cmd = QVBoxLayout()
        layout_cmd.addWidget(self.on_button, alignment= Qt.AlignmentFlag.AlignCenter)
        layout_cmd.addWidget(self.trig_off_widget)
        layout_cmd.addWidget(self.flash_off_widget)
        layout_cmd.addWidget(self.flash_on_widget)
        layout_cmd.addWidget(self.trig_shift_widget)
        layout_cmd.addWidget(self.trig_expo_widget)

        layout_log = QVBoxLayout()
        layout_log.addWidget(self.serial_logger)

        layout_app = QHBoxLayout()
        layout_app.addLayout(layout_cmd)
        layout_app.addLayout(layout_log)

        widget = QWidget()
        widget.setLayout(layout_app)
        self.setCentralWidget(widget)
    
    def on_button_clicked(self):
        if self.on_button.isChecked():
            self.flasher.on()
            self.on_button.setText("Off")
        else:
            self.flasher.off()
            self.on_button.setText("On")
    def trig_off_widget_cb(self, value):
        self.flasher.trig_off(value)
    def flash_off_widget_cb(self, value):
        self.flasher.flash_off(value)
    def flash_on_widget_cb(self, value):
        self.flasher.flash_on(value)
    def trig_shift_cb(self, value):
        self.flasher.trig_shift(value)
    def trig_expo_cb(self, value):
        self.flasher.trig_expo(value)

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print("Please use : " + sys.argv[0] + " " + "[FILENAME]")
        exit(0)

    filename = sys.argv[1]
    if not os.path.exists(filename):
        print("File doesn't exist")
        exit(0)
    
    ser = serial.Serial(filename, 115200)
    app = QApplication(sys.argv)
    
    window = MainWindow(ser)
    window.show()

    sys.exit(app.exec())
