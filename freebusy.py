import datetime
import pickle
import os.path
from googleapiclient.discovery import build
from google_auth_oauthlib.flow import InstalledAppFlow
from google.auth.transport.requests import Request

SCOPES = ['https://www.googleapis.com/auth/calendar.readonly']
HOME_DIR = '/home/pi/status'
CALENDAR_ID = ''  # you@gmail.com


def main():
    """
    Taken from the Google quickstart documentation example.
    Get credentials from https://developers.google.com/calendar/quickstart/python
    """
    creds = None
    if os.path.exists(os.path.join(HOME_DIR, 'token.pickle')):
        with open(os.path.join(HOME_DIR, 'token.pickle'), 'rb') as token:
            creds = pickle.load(token)
    # If there are no (valid) credentials available, let the user log in.
    if not creds or not creds.valid:
        if creds and creds.expired and creds.refresh_token:
            creds.refresh(Request())
        else:
            flow = InstalledAppFlow.from_client_secrets_file(
                os.path.join(HOME_DIR, 'credentials.json'), SCOPES)
            creds = flow.run_local_server(port=0)
        # Save the credentials for the next run
        with open(os.path.join(HOME_DIR, 'token.pickle'), 'wb') as token:
            pickle.dump(creds, token)

    service = build('calendar', 'v3', credentials=creds)

    # Call the Calendar API
    start = (datetime.datetime.utcnow() - datetime.timedelta(seconds=60*60*2)).isoformat() + 'Z' # 'Z' indicates UTC time
    end = (datetime.datetime.utcnow() + datetime.timedelta(seconds=60*60*24)).isoformat() + 'Z' # 'Z' indicates UTC time
    events_result = service.freebusy().query(body=dict(  # pylint: disable=no-member
        timeMin=start,
        timeMax=end,
        items=[{"id": CALENDAR_ID}])
    ).execute()
    periods = events_result['calendars'][CALENDAR_ID].get('busy', [])

    if not periods:
        print('No upcoming busy periods found.')
    output = ""
    for period in periods:
        output += f"{period['start']} {period['end']}\n"
    with open(os.path.join(HOME_DIR, "gcal_status.txt"), "w") as f:
        f.write(output)
    print(f"[{datetime.datetime.now().isoformat()}] Updated freebusy schedule")


if __name__ == '__main__':
    main()
