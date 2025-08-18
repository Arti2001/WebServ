#!/usr/bin/env python3
import os, sys, cgi, cgitb, shutil, json, time

# Show trace-backs in the browser during debugging
cgitb.enable()

# --- configuration ----------------------------------------------------------
SCRIPT_DIR   = os.path.dirname(os.path.realpath(__file__))
# Assumes 'cgi-bin' is a sibling of 'pages' and 'uploads' at the project root.
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
# The path to the uploads directory is now calculated directly and reliably.
UPLOAD_DIR = os.path.join(PROJECT_ROOT, "uploads")
MAX_BYTES    = 1000 * 1024 * 1024                     # 1 MB
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
        sys.stdout.write("Status: 500 Internal Server Error\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n\r\n")
        sys.stdout.write(f"Server Error: The configured error page could not be found at {file_path}.")
        
    except Exception as e:
        sys.stdout.write("Status: 500 Internal Server Error\r\n")
        sys.stdout.write("Content-Type: text/plain\r\n\r\n")
        sys.stdout.write(f"An unexpected server error occurred: {e}")

    sys.exit(0)

def reply_with_success_page(message: str):
    """Generates and sends a full HTML page with a styled success message."""
    html_content = f"""
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Upload Successful</title>
    <style>
        body {{
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Oxygen, Ubuntu, Cantarell, "Open Sans", "Helvetica Neue", sans-serif;
            background-color: #f5f7fa;
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            color: #333;
        }}
        .container {{
            text-align: center;
            padding: 2rem;
        }}
        .success-box {{
            background-color: #e6f9f0;
            color: #00642e;
            border: 1px solid #b3e6c9;
            padding: 2rem;
            border-radius: 8px;
            max-width: 500px;
            margin: 0 auto 2rem auto;
            box-shadow: 0 4px 6px rgba(0, 0, 0, 0.1);
        }}
        .success-box h1 {{
            margin-top: 0;
            color: #004d24;
        }}
        .links a {{
            display: inline-block;
            margin: 0 1rem;
            padding: 0.75rem 1.5rem;
            border-radius: 6px;
            text-decoration: none;
            font-weight: 600;
            transition: all 0.3s ease;
            background-color: #0a2342;
            color: #ffffff;
        }}
        .links a:hover {{
            background-color: #071a33;
            transform: translateY(-2px);
        }}
    </style>
</head>
<body>
    <div class="container">
        <div class="success-box">
            <h1>Upload Successful</h1>
            <p>{message}</p>
        </div>
        <div class="links">
            <a href="/upload.html">Upload Another File</a>
            <a href="/index.html">Back to Home</a>
        </div>
    </div>
</body>
</html>
"""
    sys.stdout.write("Status: 200 OK\r\n")
    sys.stdout.write("Content-Type: text/html\r\n")
    sys.stdout.write(f"Content-Length: {len(html_content.encode('utf-8'))}\r\n")
    sys.stdout.write("\r\n")
    sys.stdout.write(html_content)
    sys.exit(0)


def main():
    # Only POST is meaningful here
    if os.environ.get("REQUEST_METHOD", "") != "POST":
        project_root = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
        error_page_path = os.path.join(project_root, "pages", "errors", "405.html")
        reply_with_html_page("405 Method Not Allowed", error_page_path)

    # Fast sanity check: make sure the web-server told us how big the body is
    try:
        content_len = int(os.environ.get("CONTENT_LENGTH", "0"))
    except ValueError:
        content_len = 0
    if content_len == 0:
        # This case might not need a full HTML page, a simple JSON error is often fine for programmatic clients
        reply("error", "Empty request or missing CONTENT_LENGTH", "400 Bad Request")

    if content_len > MAX_BYTES:
        project_root = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
        error_page_path = os.path.join(project_root, "pages", "errors", "413.html")
        reply_with_html_page("413 Payload Too Large", error_page_path)

    # Parse multipart/form-data
    form = cgi.FieldStorage()

    if "uploadFile" not in form:
        reply("error", "No form part named 'uploadFile' found.", "400 Bad Request")

    filepart = form["uploadFile"]
    if not filepart.filename:
        reply("error", "No file selected", "400 Bad Request")

    # Build safe, unique filename
    filename = os.path.basename(filepart.filename)
    name, ext = os.path.splitext(filename)
    os.makedirs(UPLOAD_DIR, exist_ok=True)

    counter, final = 0, filename
    while os.path.exists(os.path.join(UPLOAD_DIR, final)):
        counter += 1
        final = f"{name}_{counter}{ext}"

    dest_path = os.path.join(UPLOAD_DIR, final)

    # Stream the upload to disk in one pass
    with open(dest_path, "wb") as dst:
        shutil.copyfileobj(filepart.file, dst, length=1024 * 1024)  # 1 MB chunks

    os.chmod(dest_path, 0o644)
    # Send the new HTML success page instead of JSON
    reply_with_success_page(f"Your file has been successfully uploaded as: <strong>{final}</strong>")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        # Fallback for unexpected errors
        sys.stderr.write(f"Unhandled exception in upload.py: {str(exc)}\n")
        import traceback
        traceback.print_exc(file=sys.stderr)
        project_root = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
        error_page_500 = os.path.join(project_root, "pages", "errors", "500.html")
        reply_with_html_page("500 Internal Server Error", error_page_500)
