#!/usr/bin/env python
"""
nmea-to-kml.py -- Convert NMEA GPS data to KML.

Usage:
    $ ./nmea-to-kml.py input-file.txt output-file.kml

    where:
        input-file.txt is the raw NMEA GPS data
        output-file.kml is the resulting KML file

There are 19 interpreted sentences in NMEA data.  Of these, we are currently
only interested in the GPGGA GPS fix data:

   $GPBOD - Bearing, origin to destination
   $GPBWC - Bearing and distance to waypoint, great circle
   $GPGGA - Global Positioning System Fix Data
   $GPGLL - Geographic position, latitude / longitude
   $GPGSA - GPS DOP and active satellites 
   $GPGSV - GPS Satellites in view
   $GPHDT - Heading, True
   $GPR00 - List of waypoints in currently active route
   $GPRMA - Recommended minimum specific Loran-C data
   $GPRMB - Recommended minimum navigation info
   $GPRMC - Recommended minimum specific GPS/Transit data
   $GPRTE - Routes
   $GPTRF - Transit Fix Data
   $GPSTN - Multiple Data ID
   $GPVBW - Dual Ground / Water Speed
   $GPVTG - Track made good and ground speed
   $GPWPL - Waypoint location
   $GPXTE - Cross-track error, Measured
   $GPZDA - Date & Time

   http://aprs.gids.nl/nmea
"""
import sys
import re

GPGGA = re.compile("""
    # Sample line:
    # $GPGGA,035306.200,4735.7144,N,12219.6396,W,1,8,1.42,-2.1,M,-17.3,M,,*46


    ^\$GPGGA,
    (?P<hhmmss>\d{6}(\.\d{3})),      # hhmmss = '035306.200'
    (?P<latitude>\d+\.\d+),          # latitude = '4735.7144'
    (?P<N_S>[NS]),                   # N_S = 'N'
    (?P<longitude>\d+\.\d+),         # longitude = '12219.6396'
    (?P<W_E>[WE]),                   # W_E = 'W'
    (?P<fix_qual>[012]),             # fix_qual = '1'
    (?P<num_sat>\d+),                # num_sat = '8'
    (?P<hdop>\d+(\.\d+)),            # hdop = 1.42
    (?P<altitude>(\-)?\d+(\.\d+)),M, # altitude = -2.1
    (?P<height>(\-)?\d+(\.\d+)),M,   # height = -17.3
    (?P<dgps>([!\,]+)?),             # dgps = ''
    (?P<checksum>\*\w\w)$            # checksum = '*46'
""", re.VERBOSE)
    

KML_HEADER = """<?xml version="1.0" encoding="UTF-8"?>
<kml xmlns="http://www.opengis.net/kml/2.2"
xmlns:atom="http://www.w3.org/2005/Atom"
xmlns:gx="http://www.google.com/kml/ext/2.2">
<Document>
<Placemark>
<gx:Track>
"""

KML_FOOTER = """</gx:Track>
</Placemark>
</Document>
</kml>
"""


def main():

    debug = True
    minus = {'N':'', 'S':'-', 'W':'-', 'E':''}

    if len(sys.argv) != 3:
        print __doc__
        sys.exit()

    in_filename = sys.argv[1]
    out_filename = sys.argv[2]

    kml_file = open(out_filename,'w')
    kml_file.write(KML_HEADER)

    for line in open(in_filename,'r'):

        # strip any whitespace from edges
        line = line.strip()
        if not line:
            continue

        # Try to catch corrupt lines early
        if not line.startswith('$GP'):
            if debug:
                print 'Bad line: ', line
            continue
            
        # Skip any sentence other than GPGGA
        if not line.startswith('$GPGGA'):
            continue

        # Try to match a valid GPGGA sentence
        try:
            result = GPGGA.match(line).groupdict()
        except AttributeError:
            if debug:
                print 'Bad line: ', line
            continue

        if debug:
            print '%(hhmmss)s, %(longitude)s %(W_E)s, %(latitude)s %(N_S)s, %(altitude)s' % result

        hhmmss = result['hhmmss']
        result['time_string'] = ':'.join((hhmmss[0:2], hhmmss[2:4], hhmmss[4:10]))
        result['latitude'] = minus[result['N_S']] + result['latitude']
        result['longitude'] = minus[result['W_E']] + result['longitude']
        kml_file.write('<when>T%(time_string)sZ</when>' % result)
        kml_file.write('<gx:coord>%(longitude)s %(latitude)s %(altitude)s</gx:coord>\n' % result)

    kml_file.write(KML_FOOTER)
    kml_file.close()


if __name__ == '__main__':
    main()

