Make a free/busy light for your home office, which automatically updates based on Google Calendar availability, or by manual override.

Server component (which I have running on a pi) checks GCal using a cron, and provides an endpoint that overrides can be POSTed to. The first time the freebusy script is run you'll need to provide credentials, and authenticate, it's much easier to do this locally then `scp` up your `token.pickle`.

Arduino component is written to run on an Arduino Nano 33 IOT with a NeoPixel Ring (16 LEDs). It pings the server every few seconds for updated status.