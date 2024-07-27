import math
import os
import time
from ctypes import windll, Structure, c_long, byref
from win32gui import FindWindow, GetWindowRect, IsWindowVisible, GetWindowText, EnumWindows, MoveWindow, SendMessage, GetCursorInfo
from win32con import MOUSEEVENTF_WHEEL, WHEEL_DELTA, KEYEVENTF_KEYUP, MOUSEEVENTF_LEFTDOWN, MOUSEEVENTF_LEFTUP, WM_CLOSE
from win32api import mouse_event, keybd_event, GetSystemMetrics
import win32com.client as comclt #used for pressing keyboard buttons
from win32process import GetWindowThreadProcessId
from psutil import Process, pid_exists
import getpixelcolor as pixel
from pyautogui import typewrite


class CameraGui:
    def __init__(self, index):
        self.wsh = comclt.Dispatch("WScript.Shell") #Start shell to allow for key presses
        self.index = index #Index of camera in drop-down menu (Starts at 0)
        self.enum_windows = dict()  # List of currently visible windows
        self.window_rect = dict()  # Bounding box of current window
        self.cur_pos = {"x": 0, "y": 0}  # Current absolute mouse position
        self.pos_dict = {"Camera Select": (0.1, 0.1), "Preview": (0.3, 0.3), "Record": 'r',
                               "Play": 'p', "Pause": (0.5, 0.19), "Stop": (0.6, 0.19),
                               "Load Config": 'l', "Save Seq": 's', "Reset": (0.88, 0.19),
                               "High Frame Rate": 'h', "Buffer": 'b', "Save Metadata": (0.2, 0.97),
                         "Number Pixel": (0.274, 0.192), "Select Window": (0.033, 0.8)
                         }#Position of widgets relative to window size
        self.current_hwnd = None #Current OS window object
        self.keypress_dict = {"Save seq": 's', "Load Config": 'l',
                              "Play": 'p'}

        # Get size of screen - https://stackoverflow.com/questions/3129322/how-do-i-get-monitor-resolution-in-python
        self.screen_width = GetSystemMetrics(0)  # 1280
        self.screen_height = GetSystemMetrics(1)  # 800
        self.camera_type = "None"
        self.y_shift = -0.18
        self.half_screen_w = round(self.screen_width / 2)
        self.half_screen_h = round(self.screen_height / 2)
        self.half_screen_h = round(self.half_screen_h/(1+self.y_shift))
        self.x_offset = (self.index%2) * self.half_screen_w #Calculate position of top left corner of window
        self.y_offset = round((math.floor(self.index/2)+self.y_shift) * self.half_screen_h) #Calculate position of top left corner of window
        if(index > 1): #Add an extra offset to the second row to make the "Save Metadata" button visible
            self.y_offset += round(self.y_shift * self.half_screen_h)
        self.getBkgndWindows() #List of all open windows at start of program - used to identify which window is new.
        self.startSoftware() #Start the camera software
        self.getWindowPos("Acquisition Configuration")
        self.selectCamera(self.index)
        self.pressReturn()
        time.sleep(0.5)
        self.tileWindow()
        self.resizeImage(6)
        #self.showCursorPosition()
        #self.pressButton("Record")
        #self.closeProgram()


    def __winEnumHandler(self, hwnd, ctx):  # https://stackoverflow.com/questions/55547940/how-to-get-a-list-of-the-name-of-every-open-window
        if IsWindowVisible(hwnd):
            self.enum_windows[hwnd] = GetWindowText(hwnd)


    def getWindowList(self):
        # Get list of all visible background windows
        EnumWindows(self.__winEnumHandler, None)
        return self.enum_windows.copy()

    def getBkgndWindows(self):
        self.bkgnd_windows = self.getWindowList()
    def startSoftware(self):
        # Start the camera software
        os.startfile("C:\\Program Files\\Teledyne DALSA\\Sapera\\Demos\\Binaries\\GigEMetaDataDemo.exe")
        time.sleep(1)


    def getWindowPos(self, window_name):
        global window_rect
        global current_hwnd
        # get position of camera software window
        EnumWindows(self.__winEnumHandler, None)  # Get list of all visible windows
        for i in range(3): #Try three times to find the window
            for key, value in self.enum_windows.items():
                if window_name in value:  # https://realpython.com/python-string-contains-substring/
                    for b_key, b_value in self.bkgnd_windows.items():  # check that the window isn't already a background window
                        if b_key == key:
                            break
                    else:
                        self.window_rect = GetWindowRect(key)
                        self.current_hwnd = key
                        return
            else:
                time.sleep(1)
        else:
            print("ERROR: Window Not Found!")

    def selectCamera(self, index):
        rel_pos = {"x": round(self.pos_dict["Camera Select"][0] * self.window_rect[2]), "y": round(
            self.pos_dict["Camera Select"][1] * self.window_rect[3])}  # Move to mouse position relative to top window corner

        # Move cursor to camera select and use mousewheel to select the camera
        windll.user32.SetCursorPos(rel_pos["x"] +self.window_rect[0],rel_pos["y"] +self.window_rect[
            1])  # https://stackoverflow.com/questions/1181464/controlling-mouse-with-python - move cursor to camera select postion
        mouse_event(MOUSEEVENTF_WHEEL,rel_pos["x"] +self.window_rect[0],rel_pos["y"] +self.window_rect[1], 5 * WHEEL_DELTA,
                    0)  # https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mouse_event - spin mouse wheel to top of menu list
        time.sleep(0.2)

        #Open the two nIR cameras first then the color camera
        nIR = -1
        for i in range(3):
            time.sleep(0.1)
            self.getCameraID()
            if("nIR" in self.camera_type):
                nIR += 1
            if(self.index == 0 and nIR == 0): #If camera is nIR #1
                return
            elif(self.index == 1 and nIR == 1): #If camera is nIR #2
                return
            elif(self.index == 2 and "visible" in self.camera_type): #If camera is color
                return
            mouse_event(MOUSEEVENTF_WHEEL,rel_pos["x"] +self.window_rect[0],rel_pos["y"] +self.window_rect[1],
                        -1 * WHEEL_DELTA, 0)  # https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mouse_event - spin mouse wheel to desired camera
        else:
            print("ERROR: Camera not found!")

    def pressReturn(self):
        # Press the return key to go to the main window
        keybd_event(0x0D, 0, 0, 0)
        time.sleep(0.05)
        keybd_event(0x0D, 0, KEYEVENTF_KEYUP, 0)

    def pressTab(self, count=1):
        for i in range(count):
            # Press the return key to go to the main window
            keybd_event(0x09, 0, 0, 0)
            time.sleep(0.05)
            keybd_event(0x09, 0, KEYEVENTF_KEYUP, 0)
            time.sleep(0.05)

    def typeString(self, string):
        string = str(string) #Cast variable to string
        typewrite(string)

    def tileWindow(self):
        # Resize and reposition the main demo window
        self.getWindowPos("Sapera GigE-Vision MetaData Demo (Per-Frame Metadata)") #Get hwnd for window
        MoveWindow(self.current_hwnd, self.x_offset, self.y_offset, self.half_screen_w, self.half_screen_h, True)
        self.getWindowPos("Sapera GigE-Vision MetaData Demo (Per-Frame Metadata)") #record new position
        time.sleep(0.5)

    def resizeImage(self, n_steps):
        # Resize the camera view to match the window
        pos = (round((self.half_screen_w * self.pos_dict["Preview"][0]) + self.x_offset)), round((self.half_screen_h * self.pos_dict["Preview"][1]) + self.y_offset)
        delta = WHEEL_DELTA
        if(n_steps < 0):
            delta *= -1
            n_steps *= -1
        for i in range(n_steps):
            windll.user32.SetCursorPos(pos[0], pos[
                1])  # https://stackoverflow.com/questions/1181464/controlling-mouse-with-python
            mouse_event(MOUSEEVENTF_WHEEL, pos[0], pos[1], delta,
                        0)  # https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mouse_event
            time.sleep(0.1)

    def selectCameraWindow(self):
        rel_pos = {"x": round(self.pos_dict["Select Window"][0] * self.half_screen_w + self.window_rect[0]), "y": round(
            self.pos_dict["Select Window"][1] * self.half_screen_h + self.window_rect[1])}  # Click on window if keypress is used
        windll.user32.SetCursorPos(rel_pos["x"], rel_pos["y"])  # Move mouse to button
        mouse_event(MOUSEEVENTF_LEFTDOWN, rel_pos["x"], rel_pos["y"], 0, 0)  # Click mouse button
        time.sleep(0.01)
        mouse_event(MOUSEEVENTF_LEFTUP, rel_pos["x"], rel_pos["y"], 0, 0)
        time.sleep(0.05)

    def pressButton(self, button):
        try:
            pos = self.queryMousePosition() #Get current mouse position
            rel_pos = {"x": round(self.pos_dict["Preview"][0] * self.half_screen_w + self.window_rect[0]), "y": round(
                self.pos_dict["Preview"][1] * self.half_screen_h + self.window_rect[1])} #Click on window if keypress is used
            if(type(self.pos_dict[button]) is tuple):
                rel_pos = {"x": round(self.pos_dict[button][0] * self.half_screen_w + self.window_rect[0]), "y": round(
                    self.pos_dict[button][1] * self.half_screen_h + self.window_rect[1])} # https://stackoverflow.com/questions/1181464/controlling-mouse-with-python - move cursor to camera select postion
            windll.user32.SetCursorPos(rel_pos["x"], rel_pos["y"]) #Move mouse to button
            mouse_event(MOUSEEVENTF_LEFTDOWN, rel_pos["x"], rel_pos["y"], 0, 0) #Click mouse button
            time.sleep(0.01)
            mouse_event(MOUSEEVENTF_LEFTUP, rel_pos["x"], rel_pos["y"], 0, 0)
            time.sleep(0.05)
            if (type(self.pos_dict[button]) is str):
                self.wsh.SendKeys(self.pos_dict[button]) # send the keys you want
            windll.user32.SetCursorPos(pos["x"], pos["y"])  # Move mouse to button original position

        except KeyError:
            print("ERROR: No matches found for button key: " + str(button))

    # FindWindow takes the Window Class name (can be None if unknown), and the window's display text. https://stackoverflow.com/questions/7142342/get-window-position-size-with-python
    # window_handle = FindWindow(None, "Acquisition Configuration")
    # window_rect = GetWindowRect(window_handle)

    # print(window_rect)
    #
    #
    # Get mouse position fast. https://stackoverflow.com/questions/3698635/how-to-get-the-text-cursor-position-in-windows

    def queryMousePosition(self):
        pt = POINT()
        windll.user32.GetCursorPos(byref(pt))
        return {"x": pt.x, "y": pt.y}

    def queryMouseState(self):
        cursor_info = GetCursorInfo()
        return cursor_info[1]

    def showCursorPosition(self):
        rel_pos = {}
        cur_pos = (0,0)
        while True:
            pos = self.queryMousePosition()
            if cur_pos != pos:
                cur_pos = pos
                rel_pos["x"] = (cur_pos["x"] -self.window_rect[0]) / (self.window_rect[2] - self.window_rect[0])
                rel_pos["y"] = (cur_pos["y"] -self.window_rect[1]) / (self.window_rect[3] - self.window_rect[1])
                print(rel_pos)

    def closeProgram(self): #https://stackoverflow.com/questions/49593533/how-to-close-a-window-in-python-by-name
        _, pid = GetWindowThreadProcessId(self.current_hwnd)
        process = Process(pid=pid)
        SendMessage(self.current_hwnd, WM_CLOSE, 0, 0)
        if pid_exists(pid=pid):
            process.kill()

    #Use text at the end of device name to differentiate color from nIR cameras
    def getCameraID(self):
        pixels = pixel.average(round(self.pos_dict["Number Pixel"][0] * (self.window_rect[2] - self.window_rect[0]) + self.window_rect[0]), round(
                self.pos_dict["Number Pixel"][1] * (self.window_rect[3] - self.window_rect[1]) + self.window_rect[1]), 24, 12)
        if(pixels[0] > 210):
            self.camera_type = "visible"
        else:
            self.camera_type = "nIR"

class POINT(Structure):
    _fields_ = [("x", c_long), ("y", c_long)]



