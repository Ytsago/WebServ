import time
import sys

print("Content-Type: text/html\r\n\r\n", end="") # Headers obligatoires
print("<html>Hello world</html>")
sys.stdout.flush() # Force l'envoi dans le pipe

for i in range(6):
    print("<br>Iteration " + str(i))
    sys.stdout.flush() # Force l'envoi
    time.sleep(1)