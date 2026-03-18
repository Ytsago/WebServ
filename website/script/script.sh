#!/bin/bash

HTTP header
echo "Content-Type: text/html"
echo ""

Body
echo "<html>"
echo "<head><title>Bash CGI</title></head>"
echo "<body>"
echo "<h1>Hello from Bash CGI</h1>"

echo "<h2>Request Info</h2>"
echo "<p>Method: $REQUEST_METHOD</p>"
echo "<p>Query: $QUERY_STRING</p>"

If POST, read input
if [ "$REQUEST_METHOD" = "POST" ]; then
    echo "<h2>POST Data</h2>"
    read POST_DATA
    echo "<p>$POST_DATA</p>"
fi

echo "</body>"
echo "</html>"