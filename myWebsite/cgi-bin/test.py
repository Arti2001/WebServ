#!/usr/bin/env python3
import os
import sys

print("Content-Type: text/html\r")
print("\r")
print("<html>")
print("<head><title>CGI Test</title></head>")
print("<body>")
print("<h1>CGI Environment Variables</h1>")
print("<table border=\"1\">")
for env_var in sorted(os.environ.keys()):
    print(f"<tr><td>{env_var}</td><td>{os.environ[env_var]}</td></tr>")
print("</table>")
if os.environ.get('REQUEST_METHOD') == 'POST':
    print("<h2>POST Data:</h2>")
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    if content_length > 0:
        post_data = sys.stdin.read(content_length)
        print(f"<pre>{post_data}</pre>")
print("</body>")
print("</html>")
