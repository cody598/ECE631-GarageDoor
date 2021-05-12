#! /usr/bin/env python3
# ECE 631 Spring 2021
# Author: Cody Morse 
# Final Project NFC UID get.
#

import time
import datetime
import binascii
import RPi.GPIO as GPIO
import pn532
import MQTTClients
import json

UID = ["09260994", "39979ab2", "89de1494"]
count = 0;
correct = False;
MQTTPub = MQTTClients.MQTTPusher('127.0.0.1',1883)
if __name__ == '__main__':
        try:
                oldTime = time.time()
                delayOldTime = time.time()
                PN532 = pn532.PN532_SPI(debug=False, reset=22, cs=8)
                ic, ver, rev, support = PN532.get_firmware_version()
                print('Found PN532 with firmware version: {0}.{1}'.format(ver, rev))

                # Configure PN532 to communicate with MiFare cards
                PN532.SAM_configuration()

                print('Waiting for RFID/NFC card...')
                while True: #Main Loop
                    try:
                    # Check if a card is available to read with 100ms timeout
                       uid = PN532.read_passive_target(timeout=0.1)
                       # No card is available.
                       if uid is None:
                          continue

                       if time.time() - delayOldTime >= 1:
                          uid = (str)(binascii.hexlify(uid).decode('utf-8'))
                          oldTime = time.time()
                          delayOldTime = time.time()
                          for x in UID:  
                            count+=1
                            if x == uid:
                                payload = {"ID":"Pass"}
                                x = json.dumps(payload)
                                MQTTPub.PushData("/ece631/FinalProject/UID",x)
                                print('Found card with UID: %s'%uid)
                                correct = True;
                                break
                          if count == 3 and correct == False:
                            payload = {"ID":"Denied"}
                            x = json.dumps(payload)
                            MQTTPub.PushData("/ece631/FinalProject/UID",x)
                            print('Access Denied with UID: %s'%uid)
                            count = 0
                          correct = False;
                            
                    
                    except Exception as e:
                       print("Loop Exception: %s"%e)

        except Exception as e:
                print("Main Exception: %s"%e)
        finally:
                GPIO.cleanup()
