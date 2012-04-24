"""
blackbox.py -- Simple GPS data logger for Android.

First install the latest SL4A (Scripting Layer for Android) on your Android
phone.  SL4A was started by a Googler and lets you run scripting languages
on Android phones.  Python is by far the most actively developed / supported
of the scripting languages under SL4A.

The recommended apk is available here (none of the sl4a apps are available
from Google Play):

    http://www.mithril.com.au/android/sl4a_r5x.apk

More information on SL4a here:

    http://code.google.com/p/android-scripting/

Once the sl4a_r5x.apk app is installed, run it and add a Python2 interpreter.
Next put this script in the /sdcard/sl4a/scripts/ directory.

Now from SL4A, run blackbox.py.  You will be prompted for a track name.
Be sure to pick a unique track name, as previous data with the same track
name will be overridden.

You will then be prompted for a recording duration.  Pick something reasonable
like 60 seconds.  The recording will stop automatically after the selected
duration.

Upon completion, data will be stored in the directory /sdcard/blackbox/.
There should be a <track_name>.db SQLite3 file as well as a <track_name>.csv
CSV file.

The CSV file content looks like this:

$ cat /sdcard/blackbox/track1.csv
# time, latitude, longitude, altitude, speed, accuracy
1335227994.0,47.724086386151612,-122.16345777735114,102.5999755859375,0.31275001168251038,5
1335228061.0,47.724020923487842,-122.16361217200756,106,1.1259000301361084,10
...

Where:
    time = Unix epoch time (seconds since Jan 1 1970)  (float)
    latitude = latitude in degrees (float)
    longitude = longitude in degrees (float)
    altitude = elevation in meters (float)
    speed = speed in meters/second (float)
    accuracy = GPS accuracy in meters (int)

Note that the phone's GPS sensor may not yield new data even if sampled often.
For example, you can try to sample GPS every second, but you may only get new
data every ~15 seconds.
"""

import android, time, sqlite3, os

SAMPLE_RATE = 500 # in msec
SLEEP = 1

def write_csv_file(fp, gps):
    fp.write('%(time)s,%(latitude)s,%(longitude)s,%(altitude)s,%(speed)s,%(accuracy)s\n' % gps)
    
def write_kml_file(fp, gps):
    pass

def write_db(conn, cur, gps):
    cur.execute("""
       INSERT INTO log VALUES (
           NULL, %(time)s, 
           %(latitude)s, %(longitude)s, %(altitude)s,
           %(speed)s, %(accuracy)s)""" % gps)
    conn.commit()
                                                    


droid = android.Android()
droid.startLocating()
#droid.startLocating(SAMPLE_RATE)

title = 'BlackBox'                                                
message = 'Please make sure you have GPS enabled.'
droid.dialogCreateAlert(title, message) 
droid.dialogSetPositiveButtonText('OK')
droid.dialogShow()
response = droid.dialogGetResponse().result

track_name = droid.dialogGetInput('Unique Track Name').result
BLACKBOX_DIR = '/sdcard/blackbox/'
DB_FILE_NAME  = BLACKBOX_DIR + track_name + '.db'
CSV_FILE_NAME = BLACKBOX_DIR + track_name + '.csv'
KML_FILE_NAME = BLACKBOX_DIR + track_name + '.kml'

DURATION = int(droid.dialogGetInput('Recording Time in Seconds').result)

# Create a blackbox directory if it does not yet exist
try:
    os.stat(BLACKBOX_DIR)
except OSError:
    os.mkdir(BLACKBOX_DIR)

# Delete old track data with same track_name
try:
    os.unlink(DB_FILE_NAME)
    os.unlink(CSV_FILE_NAME)
    os.unlink(KML_FILE_NAME)
except OSError:
    pass
    
#kml_file = open(KML_FILE_NAME, 'w')
csv_file = open(CSV_FILE_NAME, 'w')
csv_file.write('# time, latitude, longitude, altitude, speed, accuracy\n')

conn = sqlite3.connect(DB_FILE_NAME)
cur = conn.cursor()
cur.execute("""
    CREATE TABLE log(
        id int primary key,
        timestamp int,
        latitude float,
        longitude float,
        altitude float,
        speed int,
        accuracy int)
    """)
conn.commit()

title = 'GPS'
message = 'Waiting 10 more seconds for GPS lock.'
droid.dialogCreateSpinnerProgress(title, message)
droid.dialogShow()
time.sleep(10)  # wait and hope for a GPS lock ...
droid.dialogDismiss()

droid.makeToast('***************\n** S T A R T **\n***************')
            
start_time = time.time()
last_time = 0
while True:
    time.sleep(SLEEP)
    try:
        gps = droid.readLocation()[1]['gps']
    except KeyError:
        continue
        
    if gps['time'] == last_time:
        # no new data available yet.  skip.
        continue
    last_time = gps['time']
    gps['time'] /= 1000 # convert msec to sec
    
    write_db(conn, cur, gps)
    write_csv_file(csv_file, gps)
    message = 'LAT: %(latitude)s\nLONG: %(longitude)s\nACC: %(accuracy)s' % gps
    droid.makeToast(message)
    if time.time() - start_time > DURATION:
        break

csv_file.close()
droid.stopLocating()

title = 'Done!'                                                
message = 'Data has been stored in /sdcard/blackbox/%s.*' % track_name
droid.dialogCreateAlert(title, message) 
droid.dialogSetPositiveButtonText('OK')
droid.dialogShow()
response = droid.dialogGetResponse().result

