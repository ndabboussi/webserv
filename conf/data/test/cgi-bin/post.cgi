#!/usr/bin/env bash

# Print the required HTTP header
echo "Content-Type: text/html"
echo ""

# Read POST data from stdin
read -n "$CONTENT_LENGTH" POST_DATA

# Output a simple HTML page showing the received data
echo "<html><body>"
echo "<h1>POST CGI Test</h1>"
echo "<p><strong>Received POST data:</strong></p>"
echo "<pre>${POST_DATA}</pre>"
echo "</body></html>"