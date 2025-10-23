#!/bin/bash

# CGI requires you to print headers before any body output
echo "Content-Type: text/html"
echo ""

# Read the POST data from stdin according to CONTENT_LENGTH
read -n "$CONTENT_LENGTH" POST_DATA

# Output HTML with the POST data included
echo "<html>"
echo "<head><title>POST Test (Shell CGI)</title></head>"
echo "<body>"
echo "<h1>POST CGI Test (.sh)</h1>"
echo "<p><strong>Received POST data:</strong></p>"
echo "<pre>${POST_DATA}</pre>"
echo "</body>"
echo "</html>"
