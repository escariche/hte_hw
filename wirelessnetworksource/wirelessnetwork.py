#!/usr/bin/python
import os
import xively
import subprocess
import time
import datetime
import requests
import serial

#some defines for Xively 
FEED_ID = "1240341619"  #enter your feed ID number here
API_KEY = "mNfXgp1GJjFjRpQGAxLaPMgmhAmqJfKmCkr8tgwscuoiXJ4w"  #set your API key here

#Create mapping from transmitter ID number to names. This could be a DB some day
DEV={'1':'Bedroom1', '2':'Basement'}

DEBUG = True

 
# function to return a datastream object. This either creates a new datastream,
# or returns an existing one
def get_datastream(feed, streamName, uni):
  try:
    datastream = feed.datastreams.get(streamName)
    if DEBUG:
      print "Found existing datastream"
    return datastream
  except:
    if DEBUG:
      print "Creating datastream"
    datastream = feed.datastreams.create(streamName, unit=uni)
    return datastream

#post a number to a xively feed
def put_xively(value,streamName,uni):
    # initialize api client
    api = xively.XivelyAPIClient(API_KEY)
    feed = api.feeds.get(FEED_ID)
    streamName=streamName.replace(" ", "_")
    datastream = get_datastream(feed, streamName, uni)
    datastream.max_value = None
    datastream.min_value = None

    if DEBUG:
      print "Updating Xively feed with value: %s" % value
    datastream.current_value = value
    datastream.at = datetime.datetime.utcnow()
    try:
      datastream.update()
    except requests.HTTPError as e:
      print "HTTPError({0}): {1}".format(e.errno, e.strerror)

def run():
    while(True):
        ser = serial.Serial("/dev/ttyAMA0", 9600)
        line=ser.readline().strip()
        if DEBUG:
            print "Received input: "+line
        splitList=line.split(":")

    	#make a mapping out of the split list
        values=dict(zip(splitList[0::2],splitList[1::2]))
        #sanity check the values.  We need at least an ID and a Timestamp
        if (values.has_key('ID') and values.has_key('TS')) :
	    if (DEV.has_key(values['ID'])):
                sensor=DEV[values['ID']]
	    else:
	        sensor="DeviceID"+values['ID']
            if DEBUG:
                print "Processing data for: "+sensor
            #If there is a temperature fahrenheit (TF) value process it
            if(values.has_key('TF')):
                if DEBUG:
                    print "Posting temp fahrenheit for "+sensor
                unit={'label':"degree", 'symbol':"f"}
		try:
               	    put_xively(values['TF'], sensor+" Temperature", unit);
		except:
		    print "Error posting data.  Dropping sample."

            #If there is a temperature celsius (TC) value process it
            if(values.has_key('TC')):
                if DEBUG:
                    print "Posting temp celsius for "+sensor
                unit={'label':"degree", 'symbol':"c"}
		try:
                    put_xively(values['TC'], sensor+" Temperature", unit);
		except:
		    print "Error posting data.  Dropping sample."

            #If there is a relative humidity (RH) value process it
            if(values.has_key('RH')):
                if DEBUG:
                    print "Posting relative humidity for "+sensor
                unit={'label':"relativehumidity", 'symbol':"%"}
		try:
                    put_xively(values['RH'], sensor+" Humidity", unit);
		except:
		    print "Error posting data.  Dropping sample."

            #If you add sensor types to the sender, you can make a poster here
            #if(values.has_key('??')):
            #    if DEBUG:
            #        print "Posting ?? for "+sensor
            #    unit={'label':"??", 'symbol':"?"}
	    #	 try:
            #        put_xively(values['??'], sensor+" ???", unit)
	    #	 except:
	    #	     print "Error posting data.  Dropping sample."
	    if DEBUG:
                print
        else :
            print "Insane data recieved.  Ignoring."

run()
