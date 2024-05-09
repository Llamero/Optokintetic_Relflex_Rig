import cameraGUI
from os.path import abspath, join
import sys
from PyQt5 import QtGui, QtCore, QtWidgets, uic
from ctypes import windll
import guiMapper
from PyQt5.QtGui import QFont


class Ui(QtWidgets.QMainWindow):
    def __init__(self, app):
        self.app = app
        super(Ui, self).__init__()

        #Initialize camera software
        self.camera_list = []
        # camera_list += [cameraGUI.CameraGui(2)]
        self.camera_list += [cameraGUI.CameraGui(index) for index in range(3)]
        uic.loadUi(self.resourcePath('main_window.ui'), self)
        self.setGeometry()
        self.show()


    def resourcePath(self, relative_path):
        #Get absolute path to resource, works for dev and for PyInstaller
        try:
            # PyInstaller creates a temp folder and stores path in _MEIPASS
            base_path = sys._MEIPASS
        except Exception:
            base_path = abspath(".")

        return join(base_path, relative_path)

    def setGeometry(self): #https://stackoverflow.com/questions/39046059/pyqt-location-of-the-window
        sg = QtWidgets.QDesktopWidget().screenGeometry()
        widget = self.geometry()
        self.half_screen_w = round(sg.width() / 2)
        self.half_screen_h = round(sg.height() / 2)
        self.resize(self.half_screen_w, round(self.half_screen_h/2))
        self.move(self.half_screen_w, self.half_screen_h)
        windll.user32.SetCursorPos(round(sg.width() * 0.56), round(sg.height() * 0.56))

    def closeEvent(self, event): #https://stackoverflow.com/questions/40622095/pyqt5-closeevent-method
        close = QtWidgets.QMessageBox.question(self,
                                               "QUIT",
                                               "Are you sure want to quit?",
                                               QtWidgets.QMessageBox.Yes | QtWidgets.QMessageBox.No)
        if close == QtWidgets.QMessageBox.Yes:
            event.accept()
            for camera in self.camera_list:
                camera.closeProgram()
        else:
            event.ignore()

