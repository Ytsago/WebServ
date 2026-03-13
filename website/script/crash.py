#!/usr/bin/python3
import sys

# On écrit un peu avant de crasher
print("Content-Type: text/html\r\n\r\n", end="")
print("Quelque chose va mal se passer...")
sys.stdout.flush()

# Sortie en erreur
sys.exit(1)