color_blob.py
=============

Simple test script to detect color blobs (e.g. orange pylons
for RoboMagellan).  Given a single image or webcam video, this script will output
bounding boxes of any color blobs it detects.  Accuracy of the color blob detection
is very sensitive to the target color chosen.


**Usage**

To interactively test on a single image::

    $ ./color_blob.py -i images/cones-on-lawn.jpg 
    Bounding box  (260, 262, 46, 118)
    Bounding box  (38, 186, 8, 16)
    Bounding box  (583, 182, 23, 59)

To specify a pure red target color::

    $ ./color_blob.py -c 255,0,0 -i /path/to/another/image.jpg
    ...

To test on a live webcam video feed::

    $ ./color_blob.py -v
    ...

To pick a target color from a live webcam video::

    $ ./color_blob.py -p
    Average target color (R,G,B):  253,114,47
    ...




**Dependencies**

This was tested under Ubuntu and the argparse module used in this script assumes
Python 2.7.  This script uses OpenCV and its Python bindings.  To install the necessary 
software under Ubuntu, simply type::

    $ sudo apt-get install python-opencv

To test if the installation worked::

    $ python
    >>> import cv
    >>>                 # <--- No error here indicates success

If the webcam does not work, try a V4L (Video4Linux) compatible webcam.

