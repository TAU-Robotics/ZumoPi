#!/usr/bin/env python3
from http import server

class handler(server.BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type','text/html')
        self.end_headers()

        message = "Hello, World!"
        self.wfile.write(bytes(message, "utf8"))

with server.HTTPServer(('', 8000), handler) as server:
    server.serve_forever()