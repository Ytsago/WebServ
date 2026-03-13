#!/usr/bin/python3
import time
import sys

print("Content-Type: text/html\r\n\r\n", end="")
print("Je ne m'arrêterai jamais...")
sys.stdout.flush()

while True:
    time.sleep(1)