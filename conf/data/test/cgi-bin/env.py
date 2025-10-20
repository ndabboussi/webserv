#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import os

print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>Python CGI Test</h1>")
print("<p>Hello from Python CGI!</p>")

print("<h3>Environment Variables</h3><ul>")
for key, value in os.environ.items():
    print(f"<li>{key}: {value}</li>")
print("</ul></body></html>")
