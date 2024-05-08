import cameraGUI

camera_list = []
#camera_list += [cameraGUI.CameraGui(2)]
camera_list += [cameraGUI.CameraGui(index) for index in range(3)]

