#!/usr/bin/env python3
import os
import cgi
import mimetypes
import json

# Configuration
UPLOAD_DIR = "uploads"  # Directory where files are stored

def send_error(message):
    print("Content-Type: application/json")
    print("")
    print(json.dumps({
        "status": "error",
        "message": message
    }))

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
    # print(f"Content-Type: {content_type}")
    # print(f"Content-Length: {file_size}")
    # print(f"Content-Disposition: attachment; filename=\"{filename}\"")
    # print("") # Empty line to separate headers from body

    # Output file content
    with open(filepath, 'rb') as f:
        while True:
            chunk = f.read(8192)
            if not chunk:
                break
            os.write(1, chunk)

except Exception as e:
    send_error(f"Error retrieving file: {str(e)}") 