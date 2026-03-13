#!/usr/bin/python3
import sys

# Lecture du body envoyé par ton CgiContainer via handleWrite()
input_data = sys.stdin.read()

print("Content-Type: text/plain\r\n\r\n", end="")
print(f"J'ai recu le body suivant : {input_data}")