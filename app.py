import datetime
from dateutil.parser import parse as parse_date
import json
import os
import time

from flask import Flask, request, render_template, send_from_directory
app = Flask(__name__)

STATUS_AVAILABLE = 0
STATUS_BUSY = 1
STATUS_INTERUPTIBLE = 2
STATUS_OFF = 3
STATUS_UNKNOWN = 4

HOME_DIR = '/Users/tom/dev/status-light'

START_DAY_HOUR = 9
END_DAY_HOUR = 18

status = STATUS_UNKNOWN
override_status_until = None


@app.route('/')
def homepage():
    return render_template('index.html')


@app.route('/static/<path:path>')
def send_static(path):
    return send_from_directory('static', path)


@app.route('/status/update', methods=['POST'])
def update_status():
    global status
    global override_status_until
    status = int(request.form.get('status').strip())
    duration = int(request.form.get('duration').strip())
    if duration == 0:
        duration = 365 * 24 * 60 * 60 * 60
    override_status_until = datetime.datetime.now() + datetime.timedelta(seconds=duration)
    return str(status) + ' until ' + override_status_until.isoformat()


@app.route('/status')
def get_status():
    global status
    global override_status_until

    now = datetime.datetime.now()
    if override_status_until is not None and now < override_status_until:
        return str(status)

    is_work_hours = START_DAY_HOUR <= now.hour < END_DAY_HOUR
    is_weekend = now.weekday() in {5, 6}
    if is_weekend or not is_work_hours:
        return str(STATUS_OFF)


    now = datetime.datetime.now(datetime.timezone.utc)
    with open(os.path.join(HOME_DIR, "gcal_status.txt")) as f:
        gcal_info = f.read()
    lines = gcal_info.split("\n")
    for line in lines:
        dates = line.split()
        if not dates: continue
        dt_start = parse_date(dates[0])
        dt_end = parse_date(dates[1])
        if dt_start <= now <= dt_end:
            return str(STATUS_BUSY)
    return str(STATUS_AVAILABLE)


@app.route('/status/detail')
def get_detail():
    data = {
        'status': get_status(),
    }
    with open(os.path.join(HOME_DIR, "gcal_status.txt")) as f:
        data['cal'] = f.read()
    if override_status_until is not None and datetime.datetime.now() < override_status_until:
        data['override_status'] = status,
        data['until'] = override_status_until.isoformat()
    return json.dumps(data)

