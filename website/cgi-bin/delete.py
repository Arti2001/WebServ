#!/usr/bin/env python3
import os
import cgi
import json
import sys

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

def reply(status: str, message: str, http_code: str = "200 OK"):
    """Send a small JSON response and exit."""
    sys.stdout.write("Content-Type: application/json\r\n\r\n")
    sys.stdout.write(json.dumps({"status": status, "message": message}))
    sys.exit(0)

try:
    # Parse form data
    form = cgi.FieldStorage()

    # Get filename parameter
    if "filename" not in form:
        reply("error", "No filename specified")
        

    filename = form.getvalue("filename")
    filepath = os.path.join(UPLOAD_DIR, filename)

    # Security check: prevent directory traversal
    if '..' in filename or filename.startswith('/'):
        reply("error", "Invalid filename")
        

    # Check if file exists
    if not os.path.exists(filepath):
        reply("error", f"File '{filename}' not found")
        

    # Check if path is a regular file
    if not os.path.isfile(filepath):
        reply("error", "Not a valid file")
        

    # Delete the file
    os.remove(filepath)
    
    reply("success", f"File '{filename}' was deleted successfully")

except Exception as e:
    reply("error", f"Error deleting file: {str(e)}") 