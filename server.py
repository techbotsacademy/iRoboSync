# save as server.py

import http.server
import socketserver
import webbrowser
import os

PORT = 8000

# Change to current folder
os.chdir(os.path.dirname(os.path.abspath(__file__)))

Handler = http.server.SimpleHTTPRequestHandler

with socketserver.TCPServer(("", PORT), Handler) as httpd:

    url = f"http://localhost:{PORT}"

    print("\n====================================")
    print("YOLOv8 AI Number Plate Server Started")
    print("====================================")
    print(f"Server Running At: {url}")
    print("Press CTRL + C to stop")
    print("====================================\n")

    # Open browser automatically
    webbrowser.open(url)

    try:
        httpd.serve_forever()

    except KeyboardInterrupt:
        print("\nServer stopped")