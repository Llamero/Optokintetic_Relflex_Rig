from PyQt5 import QtGui, QtCore
from collections import OrderedDict

def initializeGuiModel(gui):
    gui_model = OrderedDict()
    gui_model["record"] = gui.recordButton
    gui_model["stop"] = gui.stopButton
    gui_model["play"] = gui.playButton
    gui_model["pause"] = gui.pauseButton
    gui_model["reset"] = gui.resetButton
    gui_model["preview"] = OrderedDict([("number", gui.previewSpinbox), ("set", gui.previewSetButton)])
    gui_model["buffer"] = OrderedDict([("number", gui.nFramesSpinbox), ("set", gui.nFramesSetButton)])
    gui_model["save"] = OrderedDict([("all", gui.saveAllButton), ("movie", gui.saveMovieButton), ("metadata", gui.saveMetadataButton)])
    gui_model["load"] = gui.loadConfigButton

    return gui_model

def initializeEvents(gui):
    gui.recordButton.clicked.connect(lambda: gui.recordEvent())
    gui.stopButton.clicked.connect(lambda: gui.stopEvent())
    gui.playButton.clicked.connect(lambda: gui.playEvent())
    gui.pauseButton.clicked.connect(lambda: gui.pauseEvent())
    gui.resetButton.clicked.connect(lambda: gui.resetEvent())
    gui.previewSetButton.clicked.connect(lambda: gui.previewEvent())
    gui.nFramesSetButton.clicked.connect(lambda: gui.bufferEvent())
    gui.saveAllButton.clicked.connect(lambda: gui.saveAllEvent())
    gui.saveMovieButton.clicked.connect(lambda: gui.saveMovieEvent())
    gui.saveMetadataButton.clicked.connect(lambda: gui.saveMetadataEvent())
    gui.loadConfigButton.clicked.connect(lambda: gui.loadConfigEvent())
