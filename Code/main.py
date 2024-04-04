import ctypes
import os
import time
from ctypes import windll, Structure, c_long, byref
from win32gui import FindWindow, GetWindowRect, IsWindowVisible, GetWindowText, EnumWindows, MoveWindow
from win32con import MOUSEEVENTF_WHEEL, WHEEL_DELTA, KEYEVENTF_KEYUP
from win32api import mouse_event, keybd_event, GetSystemMetrics

enum_windows = dict() #List of currently visible windows
window_rect = dict() #Bounding box of current window
cur_pos = {"x": 0, "y": 0} #Current absolute mouse position
rel_pos = {"x": 0, "y": 0} #Move to mouse position relative to top window corner
setup_pos_dict = {"Camera": (0.2, 0.15)}
current_hwnd = None;

#Get size of screen - https://stackoverflow.com/questions/3129322/how-do-i-get-monitor-resolution-in-python
screen_width = GetSystemMetrics(0) #1280
screen_height = GetSystemMetrics(1) #800
print(screen_width)
print(screen_height)
half_screen_w = round(screen_width/2)
half_screen_h = round(screen_height/2)
def winEnumHandler( hwnd, ctx ): #https://stackoverflow.com/questions/55547940/how-to-get-a-list-of-the-name-of-every-open-window
    if IsWindowVisible( hwnd ):
        enum_windows[hwnd] = GetWindowText( hwnd )

#Get list of all visible background windows
EnumWindows( winEnumHandler, None )
bkgnd_windows = enum_windows.copy()

#Start the camera software
os.startfile("C:\\Program Files\\Teledyne DALSA\\Sapera\\Demos\\Binaries\\GigEMetaDataDemo.exe")
time.sleep(1)

def getWindowPos(window_name):
    global window_rect
    global current_hwnd
    #get position of camera software window
    EnumWindows( winEnumHandler, None ) #Get list of all visible windows
    for key, value in enum_windows.items():
        if value == window_name:
            for b_key, b_value in bkgnd_windows.items(): #check that the window isn't already a background window
                if b_key == key:
                    break
            else:
                window_rect = GetWindowRect(key)
                current_hwnd = key
                return
    else:
        print("ERROR: Window Not Found!")
getWindowPos("Acquisition Configuration")
rel_pos["x"] = round(setup_pos_dict["Camera"][0]*window_rect[2])
rel_pos["y"] = round(setup_pos_dict["Camera"][1]*window_rect[3])

#Move cursor to camera select and use mousewheel to select the camera
ctypes.windll.user32.SetCursorPos(rel_pos["x"] + window_rect[0], rel_pos["y"] + window_rect[1]) #https://stackoverflow.com/questions/1181464/controlling-mouse-with-python
mouse_event(MOUSEEVENTF_WHEEL, rel_pos["x"] + window_rect[0], rel_pos["y"] + window_rect[1], 2*WHEEL_DELTA, 0) #https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mouse_event

#Press the rerturn key to go to the main window
keybd_event(0x0D, 0, 0, 0)
time.sleep(0.05)
keybd_event(0x0D, 0, KEYEVENTF_KEYUP, 0)

quit()

#Resize and reposition the main demo window
time.sleep(0.5)
getWindowPos("Sapera GigE-Vision MetaData Demo (Per-Frame Metadata)")
offset = -250
MoveWindow(current_hwnd, offset, offset, half_screen_w-offset, half_screen_h-offset, True)
time.sleep(0.5)

#Resize the camera view to match the window
pos = (round(half_screen_w*0.5), round(half_screen_h*0.5))
for i in range(5):
    ctypes.windll.user32.SetCursorPos(pos[0], pos[1]) #https://stackoverflow.com/questions/1181464/controlling-mouse-with-python
    mouse_event(MOUSEEVENTF_WHEEL, pos[0], pos[1], -WHEEL_DELTA, 0) #https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-mouse_event
    time.sleep(0.1)



quit()
print(bkgnd_windows)
print(window_rect)
print("Done")

# FindWindow takes the Window Class name (can be None if unknown), and the window's display text. https://stackoverflow.com/questions/7142342/get-window-position-size-with-python
#window_handle = FindWindow(None, "Acquisition Configuration")
#window_rect = GetWindowRect(window_handle)

print(window_rect)


#Get mouse position fast. https://stackoverflow.com/questions/3698635/how-to-get-the-text-cursor-position-in-windows
class POINT(Structure):
    _fields_ = [("x", c_long), ("y", c_long)]

def queryMousePosition():
    pt = POINT()
    windll.user32.GetCursorPos(byref(pt))
    return { "x": pt.x, "y": pt.y}

while True:
    pos = queryMousePosition()
    if cur_pos != pos:
        cur_pos = pos
        rel_pos["x"] = (cur_pos["x"] - window_rect[0])/window_rect[2]
        rel_pos["y"] = (cur_pos["y"] - window_rect[1])/window_rect[3]

        print(rel_pos)
