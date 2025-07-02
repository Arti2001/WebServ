#!/usr/bin/env python3
import os
import cgi
import json

# Configuration
SCRIPT_DIR   = os.path.dirname(os.path.realpath(__file__))
UPLOAD_DIR = os.path.join(
    SCRIPT_DIR, 
    os.environ.get("UPLOAD_DIR")
)

def send_response(status, message):
    print("Content-Type: application/json")
    print("")
    print(json.dumps({
        "status": status,
        "message": message
    }))

try:
    # Parse form data
    form = cgi.FieldStorage()

    # Get filename parameter
    if "filename" not in form:
        send_response("error", "No filename specified")
        exit()

    filename = form.getvalue("filename")
    filepath = os.path.join(UPLOAD_DIR, filename)

    # Security check: prevent directory traversal
    if '..' in filename or filename.startswith('/'):
        send_response("error", "Invalid filename")
        exit()

    # Check if file exists
    if not os.path.exists(filepath):
        send_response("error", f"File '{filename}' not found")
        exit()

    # Check if path is a regular file
    if not os.path.isfile(filepath):
        send_response("error", "Not a valid file")
        exit()

    # Delete the file
    os.remove(filepath)
    
    send_response("success", f"File '{filename}' was deleted successfully")

except Exception as e:
    send_response("error", f"Error deleting file: {str(e)}") 