#!/usr/bin/env python3
import os
import cgi
import cgitb
import json

# Enable CGI error tracking
cgitb.enable()

# Configuration
UPLOAD_DIR = "uploads"  # Directory to store uploaded files
MAX_FILE_SIZE = 50 * 1024 * 1024  # 50MB max file size

def send_response(status, message):
    print("Content-Type: application/json")
    print("")  # Empty line to separate headers from body
    print(json.dumps({
        "status": status,
        "message": message
    }))

try:
    # Ensure upload directory exists
    if not os.path.exists(UPLOAD_DIR):
        os.makedirs(UPLOAD_DIR)

    # Parse the form data
    form = cgi.FieldStorage()

    # Check if file was uploaded
    if "uploadFile" not in form:
        send_response("error", "No file was uploaded")
        exit()

    fileitem = form["uploadFile"]

    # Check if file is empty
    if not fileitem.filename:
        send_response("error", "No file selected")
        exit()

    # Get file size
    file_size = len(fileitem.file.read())
    fileitem.file.seek(0)  # Reset file pointer

    # Check file size
    if file_size > MAX_FILE_SIZE:
        send_response("error", f"File too large. Maximum size is {MAX_FILE_SIZE/1024/1024}MB")
        exit()

    # Create safe filename
    filename = os.path.basename(fileitem.filename)
    filepath = os.path.join(UPLOAD_DIR, filename)

    # Check if file already exists
    counter = 1
    name, ext = os.path.splitext(filename)
    while os.path.exists(filepath):
        filename = f"{name}_{counter}{ext}"
        filepath = os.path.join(UPLOAD_DIR, filename)
        counter += 1

    # Save the file
    with open(filepath, 'wb') as f:
        f.write(fileitem.file.read())

    # Set file permissions
    os.chmod(filepath, 0o644)

    send_response("success", f"File '{filename}' was uploaded successfully")

except Exception as e:
    send_response("error", f"Upload failed: {str(e)}") 