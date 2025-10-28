#!/bin/bash

# CGI requires you to print headers before any body output
echo "Content-Type: text/html"
echo ""

# Output HTML with the POST data included
echo "<html>"
echo "<head><title>GET Test (Shell CGI)</title></head>"
echo "<body>"
echo "<h1>GET CGI Test (.sh)</h1>"
echo "<p><strong>Hello from BASH Test <3 </strong></p>"
echo "</body>"
echo "</html>"
