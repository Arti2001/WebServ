import os, sys, cgi, cgitb, shutil, json

print("Content-Type: text/plain\n")
for k, v in os.environ.items():
    print(f"{k}: {v}")



def reply(status: str, message: str, http_code: str = "200 OK"):
    """Send a small JSON response and exit."""
    sys.stdout.write(f"Status: {http_code}\r\n")
    sys.stdout.write("Content-Type: application/json\r\n\r\n")
    sys.stdout.write(json.dumps({"status": status, "message": message}))
    sys.exit(0)

form = cgi.FieldStorage()
reply("keys form", list(form.keys()))
print("Content-Type: text/plain\n")
for key in form:
    print(f"Field: {key}, Filename: {getattr(form[key], 'filename', None)}")




# check if the file has been uploaded
if fileitem.filename:
    fn = os.path.basename(fileitem.filename)
    with open(fn, 'wb') as f:
        f.write(fileitem.file.read())

    print(f'{{"status": "success", "filename": "{fn}"}}')
else:
    print('{"status": "error", "message": "No file uploaded."}')