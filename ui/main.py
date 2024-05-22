import sys
from PyQt6.QtWidgets import QApplication, QMainWindow
import sys
from PyQt5.QtWidgets import QApplication, QMainWindow, QWidget, QHBoxLayout
from PyQt5.QtGui import QPalette, QColor
import serial

serialPico = None

def send_cmd(serial, cmd):
    for c in cmd:
        serial.write(c.encode())
    
    serial.write(";".encode())
    
    s = serialPico.read(120)
    print(s.decode())

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

        layout.addWidget(Color('red'))
        layout.addWidget(Color('green'))
        layout.addWidget(Color('blue'))

        widget = QWidget()
        widget.setLayout(layout)
        self.setCentralWidget(widget)

if __name__ == "__main__":
    serialPico = serial.Serial('/dev/ttyACM0', 115200)
    send_cmd(serialPico, "trig.en:0;")
    serialPico.close()
    exit(0)

    app = QApplication(sys.argv)

    window = MainWindow()
    window.show()

    sys.exit(app.exec())
