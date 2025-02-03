# Optokinetic Reflex Behavior Rig
 This is an open-source behavior rig designed to accurately the optokinetic reflex in mice.  This design is a derivative of the following designs:
 https://pmc.ncbi.nlm.nih.gov/articles/PMC10023158/
 https://www.jneurosci.org/content/37/5/1102
 
 The development of this scope and illuminator was funded by the following grant: NIH P30EY003176
 
 ![]([Images/OKR%20Rig.jpg)
 
## How to use the CAD files
The CAD files can be opened using FreeCAD: https://www.freecadweb.org/downloads.php  Unless otherwise specified, all the part numbers listed in the CAD file are Thorlabs part numbers.
 ![]([Images/OKR%20rig%20CAD.png)
 
## Controlling the LEDs
The green (l-cone) and UV (s-cone) 27 amp LEDs are directly controlled via the RGB LED control channels on the EKB DLP.  These 3v3 outputs are shifted to 12V with a gate driver, which is then used to switch mosfets in series with the LEDs.  Ballast resistors control the current.

All circuit design files can be found in the KiCAD filder, and opened using KiCAD: https://www.kicad.org/download/
