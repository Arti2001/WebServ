#!/usr/bin/env python3
import os
import cgi
import mimetypes
import json
import sys

# Configuration
SCRIPT_DIR   = os.path.dirname(os.path.realpath(__file__))
EXEC_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, "../../"))
UPLOAD_DIR = os.path.join(EXEC_DIR, os.environ.get("UPLOAD_DIR"))   # absolute path is safer

def send_error(message):
    body = json.dumps({
        "status": "error",
        "message": message
    }).encode('utf-8')
    sys.stdout.buffer.write(b"Content-Type: application/json\r\n")
    sys.stdout.buffer.write(b"Content-Length: %d\r\n" % len(body))
    sys.stdout.buffer.write(b"\r\n")
    sys.stdout.buffer.write(body)

try:
    # Parse query parameters
    form = cgi.FieldStorage()

    # Get filename parameter
    if "filename" not in form:
        send_error("No filename specified")
        exit()

    filename = form.getvalue("filename")
    filepath = os.path.join(UPLOAD_DIR, filename)

    # Security check: prevent directory traversal
    if '..' in filename or filename.startswith('/'):
        send_error("Invalid filename")
        exit()

    # Check if file exists
    if not os.path.exists(filepath):
        send_error(f"File '{filename}' not found")
        exit()

    # Check if path is a regular file
    if not os.path.isfile(filepath):
        send_error("Not a valid file")
        exit()

    # Get file size
    file_size = os.path.getsize(filepath)

    # Determine content type
    # content_type, encoding = mimetypes.guess_type(filepath)
    # if content_type is None:
    #     content_type = "application/octet-stream"
    content_type = "application/octet-stream"

    # Output headers
    sys.stdout.buffer.write(f"Content-Type: {content_type}\r\n".encode("ascii"))
    sys.stdout.buffer.write(f"Content-Length: {file_size}\r\n".encode("ascii"))
    sys.stdout.buffer.write(f"Content-Disposition: attachment; filename=\"{filename}\"\r\n".encode("ascii"))
    sys.stdout.buffer.write(b"\r\n") # Empty line to separate headers from body

    # Output file content
    with open(filepath, 'rb') as f:
        while True:
            chunk = f.read(8192)
            if not chunk:
                break
            sys.stdout.buffer.write(chunk)

except Exception as e:
    send_error(f"Error retrieving file: {str(e)}")