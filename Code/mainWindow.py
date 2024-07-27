import collections
import time
import cameraGUI
from os.path import abspath, join
import sys
from PyQt5 import QtWidgets, uic
from ctypes import windll
import guiMapper
from collections import OrderedDict

class Ui(QtWidgets.QMainWindow):
    def __init__(self, app):
        self.app = app
        super(Ui, self).__init__()

        #Initialize camera software
        self.camera_list = []
        # camera_list += [cameraGUI.CameraGui(2)]
        self.camera_list += [cameraGUI.CameraGui(index) for index in range(3)]
        uic.loadUi(self.resourcePath('main_window.ui'), self)
        self.gui_model = guiMapper.initializeGuiModel(self)
        guiMapper.initializeEvents(self)
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

    def recordEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Reset")
            camera.pressButton("Record")
    def stopEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Stop")
    def playEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Play")
    def pauseEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Pause")
    def resetEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Reset")
    def previewEvent(self):
        value = self.getValue(self.gui_model["preview"]["number"])
        print(value)
        for camera in self.camera_list:
            camera.pressButton("Preview")
    def bufferEvent(self):
        value = self.getValue(self.gui_model["buffer"]["number"])
        for camera in self.camera_list:
            camera.getBkgndWindows() #Create list of currently open windows
            camera.selectCameraWindow() #Select GUI for current camera
            stdby_cursor = camera.queryMouseState() #Get normal cursor handle number - this will be used to tell when cursor is busy
            camera.pressButton("Buffer")
            time.sleep(0.5)
            camera.pressTab(1)
            camera.typeString(value)
            camera.pressTab(4)
            camera.pressReturn()
            cur_cursor = None
            time.sleep(0.3)
            camera.selectCameraWindow()  # Return cursor to same position as before
            while cur_cursor != stdby_cursor:
                camera.selectCameraWindow()  # Select GUI for current camera
                cur_cursor = camera.queryMouseState()
                time.sleep(0.1)

    def saveAllEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Save Seq")
            camera.pressButton("Save Metadata")
    def saveMovieEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Save Seq")
            camera.pressButton("Save Metadata")
    def saveMetadataEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Save Seq")
            camera.pressButton("Save Metadata")
    def loadConfigEvent(self):
        for camera in self.camera_list:
            camera.pressButton("Load Config")

    def getValue(self, widget):
        if isinstance(widget, QtWidgets.QLineEdit):
            return widget.text()
        elif isinstance(widget, QtWidgets.QRadioButton) or isinstance(widget, QtWidgets.QCheckBox) or isinstance(
                    widget, QtWidgets.QPushButton):
            return widget.isChecked()
        elif isinstance(widget, QtWidgets.QSpinBox) or isinstance(widget, QtWidgets.QDoubleSpinBox) or isinstance(
                widget, QtWidgets.QSlider) or isinstance(widget, QtWidgets.QDial):
            return widget.value()
        elif isinstance(widget, QtWidgets.QToolBox):
            return widget.itemText(widget.currentIndex())
        elif isinstance(widget, list):
            if isinstance(widget[0], QtWidgets.QRadioButton) or isinstance(widget[0], QtWidgets.QPushButton):
                for element in widget:
                    if element.isChecked():
                        return element.text()

        elif isinstance(widget, QtWidgets.QTabWidget):
            return widget.tabText(widget.currentIndex())
        elif isinstance(widget, QtWidgets.QTableWidget):
            pass
        elif isinstance(widget, str) or widget is None:
            return str(widget)

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

    def disableWidgets(self, widget):
        self.gui_model["buffer"]["set"].setEnabled(False)
    def enableWidgets(self, widget):
        self.gui_model["buffer"]["set"].setEnabled(True)