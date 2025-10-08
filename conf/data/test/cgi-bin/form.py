#!/usr/bin/env python3
# -*- coding: utf-8 -*-

import cgi

form = cgi.FieldStorage()

print("Content-Type: text/html\n")
print("<html><body>")
print("<h1>Form Result</h1>")

if "name" in form:
    print(f"<p>Hello, {form.getvalue('name')}!</p>")
else:
    print("<p>No name provided.</p>")

print("</body></html>")
