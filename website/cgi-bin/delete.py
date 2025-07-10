#!/usr/bin/env python3
import os
import cgi
import json
import sys
import cgitb
from urllib.parse import parse_qs

# --- Configuration ----------------------------------------------------------
SCRIPT_DIR = os.path.dirname(os.path.realpath(__file__))
# Assumes 'cgi-bin' is a sibling of 'pages' and 'uploads' at the project root.
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
# The path to the uploads directory is now calculated directly and reliably.
UPLOAD_DIR = os.path.join(PROJECT_ROOT, "uploads")
# ---------------------------------------------------------------------------

def reply(status: str, message: str, http_code: str = "200 OK"):
    """Send a small JSON response and exit."""
    sys.stdout.write(f"Status: {http_code}\r\n")
    sys.stdout.write("Content-Type: application/json\r\n\r\n")
    sys.stdout.write(json.dumps({"status": status, "message": message}))
    sys.exit(0)

def reply_with_html_page(http_code_with_message: str, file_path: str):
    """Sends a full HTML page as a response and exits."""
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            html_content = f.read()
        
        sys.stdout.write(f"Status: {http_code_with_message}\r\n")
        sys.stdout.write("Content-Type: text/html\r\n")
        sys.stdout.write(f"Content-Length: {len(html_content.encode('utf-8'))}\r\n")
        sys.stdout.write("\r\n")
        sys.stdout.write(html_content)
        
    except FileNotFoundError:
        # If the custom error page isn't found, send a plain text error.
        sys.stdout.write("Status: 500 Internal Server Error\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n\r\n")
        sys.stdout.write(f"Server Error: The configured error page could not be found at {file_path}.")
        
    except Exception as e:
        sys.stdout.write("Status: 500 Internal Server Error\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n\r\n")
        sys.stdout.write(f"An unexpected server error occurred: {e}")

    sys.exit(0)

def get_error_page_path(error_code: int) -> str:
    """Constructs the absolute path to an error page."""
    return os.path.join(PROJECT_ROOT, "pages", "errors", f"{error_code}.html")

def main():
    """Main logic for handling file deletion."""
    try:
        # Using DELETE and reading from the query string
        if os.environ.get("REQUEST_METHOD") != "DELETE":
            error_page_405 = get_error_page_path(405)
            reply_with_html_page("405 Method Not Allowed", error_page_405)
            return

        # Manually parse the query string from the environment variable
        query_string = os.environ.get("QUERY_STRING", "")
        parsed_query = parse_qs(query_string)
        
        # Get the filename from the parsed dictionary
        filename = parsed_query.get('filename', [None])[0]

        if not filename:
            error_page_400 = get_error_page_path(400)
            reply_with_html_page("400 Bad Request", error_page_400)
            return

        # Security check: prevent directory traversal attacks
        if '..' in filename or filename.startswith('/'):
            error_page_400 = get_error_page_path(400)
            reply_with_html_page("400 Bad Request", error_page_400)
            return

        filepath = os.path.join(UPLOAD_DIR, os.path.basename(filename))

        # Verify the final path is within the intended upload directory
        if not os.path.abspath(filepath).startswith(os.path.abspath(UPLOAD_DIR)):
            error_page_400 = get_error_page_path(400)
            reply_with_html_page("400 Bad Request", error_page_400)
            return

        if not os.path.exists(filepath) or not os.path.isfile(filepath):
            error_page_404 = get_error_page_path(404)
            reply_with_html_page("404 Not Found", error_page_404)
            return

        # Delete the file
        os.remove(filepath)

        # Send a successful JSON response
        reply("success", f"File '{filename}' was deleted successfully", "200 OK")

    except Exception as e:
        # Fallback for any unexpected errors during execution
        sys.stderr.write(f"Unhandled exception in delete.py: {str(e)}\n")
        import traceback
        traceback.print_exc(file=sys.stderr)
        error_page_500 = get_error_page_path(500)
        reply_with_html_page("500 Internal Server Error", error_page_500)

# The main try/except is no longer needed because cgitb handles all exceptions
# and provides a much more detailed report.
main()


