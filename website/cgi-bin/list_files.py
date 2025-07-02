#!/usr/bin/env python3

import os
import json

print("Content-Type: application/json\r\n\r\n")

SCRIPT_DIR   = os.path.dirname(os.path.realpath(__file__))
UPLOAD_DIR = os.path.join(
    SCRIPT_DIR, 
    os.environ.get("UPLOAD_DIR")
)


try:
    files = os.listdir(UPLOAD_DIR)
    files = [f for f in files if os.path.isfile(os.path.join(UPLOAD_DIR, f))]
    print(json.dumps({"status": "success", "files": files}))
except Exception as e:
    print(json.dumps({"status": "error", "message": str(e)}))
