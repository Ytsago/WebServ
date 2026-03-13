#!/usr/bin/python3
import os
import signal
import sys

print("Content-Type: text/html\r\n\r\n", end="")
print("Je vais recevoir un signal SIGSEGV...")
sys.stdout.flush()

# Le processus s'envoie un signal de segmentation fault
os.kill(os.getpid(), signal.SIGSEGV)