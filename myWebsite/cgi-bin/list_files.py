#!/usr/bin/env python3

import os
import json

upload_dir = "uploads"  # Adjust this to your actual uploads path
try:
    files = os.listdir(upload_dir)
    files = [f for f in files if os.path.isfile(os.path.join(upload_dir, f))]
    print(json.dumps({"status": "success", "files": files}))
except Exception as e:
    print(json.dumps({"status": "error", "message": str(e)}))
