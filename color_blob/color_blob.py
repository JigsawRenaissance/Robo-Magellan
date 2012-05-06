#!/usr/bin/env python
"""
color_blob.py
=============

Simple test script to detect color blobs (e.g. orange pylons for RoboMagellan).
Given a single image or webcam video, this script will output bounding boxes
of any color blobs it detects.  Accuracy of the color blob detection is very
sensitive to the target color chosen.

Usage:

    To interactively test on a single image:
    $ ./color_blob.py -i test_images/cones-on-lawn.jpg 
    Bounding box:  (260, 262, 46, 118)
    Bounding box:  (38, 186, 8, 16)
    Bounding box:  (583, 182, 23, 59)

    To specify a pure red target color:
    $ ./color_blob.py -c 255,0,0 -i /path/to/another/image.jpg
    ...

    To test on a live webcam video feed:
    $ ./color_blob.py -v
    ...

    To pick a target color from a live webcam video:
    $ ./color_blob.py -p
    Average target color (R,G,B):  253,114,47
    ...


Dependencies

This was tested under Ubuntu and the argparse module used in this script
assumes Python 2.7.  This script uses OpenCV and its Python bindings.  To
install the necessary software under Ubuntu, simply type:

    $ sudo apt-get install python-opencv

To test if the installation worked:

    $ python
    >>> import cv
    >>>                 # <--- No error here indicates success

If the webcam does not work, try a V4L (Video4Linux) compatible webcam.
"""

import cv
import sys
import time

WINDOW_NAME = 'Color Blob Test'
LINE_COLOR = cv.CV_RGB(255,255,0)
TEXT_COLOR = cv.CV_RGB(255,255,0)
BOX_COLOR = cv.CV_RGB(255,0,0)
AREA_THRESHOLD = 200
COLOR_THRESHOLD = 50 #70

args = None


def area(rect):
    """
    Compute the area of a rectangle "rect",
    where "rect" is a 4-element list:

    [x, y, dx, dy]
    """
    return rect[2] * rect[3]


def show_image(label, img):
    """
    Display image "img" with a "label".
    """
    img_copy = cv.CloneImage(img)
    text_font = cv.InitFont(cv.CV_FONT_HERSHEY_COMPLEX, .5, .5, 0.0, 1, cv.CV_AA )
    cv.PutText(img_copy, label, (5, 15), text_font, TEXT_COLOR)
    if args.video:
        cv.PutText(img_copy, 'Press ESC key to exit', (220, 440), text_font, TEXT_COLOR)
        cv.ShowImage(WINDOW_NAME, img_copy)

    elif args.picker:
        cv.PutText(img_copy, 'Center ROI box over target color', (220, 440), text_font, TEXT_COLOR)
        cv.PutText(img_copy, 'Press ESC key when ready', (220, 470), text_font, TEXT_COLOR)
        cv.ShowImage(WINDOW_NAME, img_copy)

    else:
        cv.PutText(img_copy, 'Press any key to continue', (220, 440), text_font, TEXT_COLOR)
        cv.ShowImage(WINDOW_NAME, img_copy)
        cv.WaitKey()


def process_image(img, target_color_img, dst):
    """
    This is the core color blob detection routine.
    Parameters:
        img = source image
        target_color_image = image where all pixels are the target color
        dst = destination image after processing
        interactive = flag to determine single image mode vs live video mode
    """
    interactive = not (args.video or args.picker)

    src = cv.CloneImage(img)
    if interactive: show_image('source', img)

    # Subtract target color from image
    cv.AbsDiff(img, target_color_img, img)
    if interactive: show_image('diff', img)

    # Convert from RGB to HSV and extract the Value channel
    cv.CvtColor(img, img, cv.CV_BGR2HSV)
    cv.Split(img, None, None, dst, None)
    if interactive: show_image('val', dst)

    # Dilate and erode to get object blobs to reduce noise
    cv.Dilate(dst, dst, None, 1)
    if interactive: show_image('dilated', dst)

    cv.Erode(dst, dst, None, 1)
    if interactive: show_image('eroded', dst)

    # Convert to binary image using threshold
    cv.Threshold(dst, dst, COLOR_THRESHOLD, 255, cv.CV_THRESH_BINARY_INV)
    if interactive: show_image('threshold', dst)

    # Obtain contours around blobs
    storage = cv.CreateMemStorage(0)
    contour = cv.FindContours(dst, storage, cv.CV_RETR_TREE, cv.CV_CHAIN_APPROX_SIMPLE)

    # Compute the bounding box around each contour
    while contour:
        # Draw contour
        cv.DrawContours(src, contour, LINE_COLOR, LINE_COLOR, 0, 0, 0, (0,0))

        # Draw rectangles
        rect = cv.BoundingRect(list(contour))
        p1 = (rect[0], rect[1])
        p2 = (rect[0] + rect[2], rect[1] + rect[3])
        if area(rect) > AREA_THRESHOLD:
            cv.Rectangle(src, p1, p2, BOX_COLOR, 1)
            print 'Bounding box: ', rect

        contour = contour.h_next()

    show_image('bounding boxes', src)


def main():
    import argparse
    global args

    parser = argparse.ArgumentParser()

    parser.add_argument('-i', action='store', dest='filename',
        default=None, help='single image mode (interactive)')

    parser.add_argument('-v', action='store_true', dest='video',
        default=False, help='live webcam video mode')

    parser.add_argument('-p', action='store_true', dest='picker',
        default=False, help='color picker for live webcam')

    parser.add_argument('-c', action='append', dest='rgb',
        default=None, help='RGB color triplet, e.g.: 255,255,0')

    args = parser.parse_args()
    
    TARGET_COLOR = cv.CV_RGB(253,114,47)     # indoor natural day light orange
    #TARGET_COLOR = cv.CV_RGB(232,15,8)      # indoor flourescent light orange
    #TARGET_COLOR = cv.CV_RGB(253,146,95)    # outdoor overcast orange
    if args.rgb:
        # If color was specified in the command line, use it
        r, g, b = args.rgb[0].split(',')
        r, g, b = int(r), int(g), int(b)
        TARGET_COLOR = cv.CV_RGB(r, g, b)
        
    if args.video or args.picker:
        # Live webcam video mode
        capture = cv.CaptureFromCAM(0)
        src = cv.QueryFrame(capture)
        if not src:
            print 'Unable to capture video.  Is the webcam hooked up?'
            sys.exit()

    elif args.filename:
        # Single image mode (interactive)
        src = cv.LoadImage(args.filename, cv.CV_LOAD_IMAGE_COLOR)

    else:
        print __doc__
        sys.exit()

    cv.NamedWindow(WINDOW_NAME, cv.CV_WINDOW_AUTOSIZE)

    # Pre-allocate result image
    dst = cv.CreateImage(cv.GetSize(src),8,1)
    roi = cv.CreateImage((100,100),8,3)

    # Create an image filled with the target color
    target_color_img = cv.CreateImage(cv.GetSize(src),8,3)
    cv.Rectangle(target_color_img, (0,0), (639,479), TARGET_COLOR, cv.CV_FILLED)

    if args.video:
        # Live webcam mode
        while True:
            src = cv.QueryFrame(capture)
            process_image(src, target_color_img, dst)
            if cv.WaitKey(10) == 27:
                break
    elif args.picker:
        # Color picker mode (implies webcam mode)
        while True:
            src = cv.QueryFrame(capture)
            cv.Rectangle(src, (270,190), (370,290), BOX_COLOR, 1)
            show_image("Color Picker Mode", src)
            if cv.WaitKey(10) == 27:
                break
            roi = cv.GetSubRect(src,(271,191,99,99))
            avg_color = cv.Avg(roi)
            print "Average target color: %d,%d,%d" % (int(avg_color[2]), int(avg_color[1]), int(avg_color[0]))
    else:
        # Single image mode (interactive)
        process_image(src, target_color_img, dst)

    cv.DestroyWindow(WINDOW_NAME)

if __name__ == '__main__':
    main()


