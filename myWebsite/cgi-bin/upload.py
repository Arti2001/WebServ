#!/usr/bin/env python3
import os, sys, cgi, cgitb, shutil, json

sys.stderr.write(f"CGI got {os.environ.get('CONTENT_LENGTH')} bytes, "
                 f"stdin has {len(sys.stdin.buffer.peek())} bytes\n")

# Show trace-backs in the browser during debugging
cgitb.enable()
print(os.environ, file=sys.stderr)
# --- configuration ----------------------------------------------------------
SCRIPT_DIR   = os.path.dirname(os.path.realpath(__file__))
UPLOAD_DIR   = os.path.join(SCRIPT_DIR, "uploads")   # absolute path is safer
MAX_BYTES    = 50 * 1024 * 1024                     # 50 MiB
# ---------------------------------------------------------------------------

def reply(status: str, message: str, http_code: str = "200 OK"):
    """Send a small JSON response and exit."""
    sys.stdout.write(f"Status: {http_code}\r\n")
    sys.stdout.write("Content-Type: application/json\r\n\r\n")
    sys.stdout.write(json.dumps({"status": status, "message": message}))
    sys.exit(0)


def main():
    # Only POST is meaningful here
    if os.environ.get("REQUEST_METHOD", "") != "POST":
        reply("error", "Only POST uploads are accepted", "405 Method Not Allowed")

    # Fast sanity check: make sure the web-server told us how big the body is
    try:
        content_len = int(os.environ.get("CONTENT_LENGTH", "0"))
    except ValueError:
        content_len = 0
    if content_len == 0:
        reply("error", "Empty request or missing CONTENT_LENGTH")

    if content_len > MAX_BYTES:
        reply("error", f"Payload exceeds {MAX_BYTES//1024//1024} MB limit")

    # Parse multipart/form-data
    form = cgi.FieldStorage()

    if "uploadFile" not in form:
        reply("error", "No form part named 'uploadFile' found")

    filepart = form["uploadFile"]
    if not filepart.filename:
        reply("error", "No file selected")

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
    reply("success", f"Uploaded as {final}")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:        # Fail safe and verbosely
        reply("error", f"Exception: {exc}", "500 Internal Server Error")
