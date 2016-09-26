#!/usr/bin/python
from BaseHTTPServer import BaseHTTPRequestHandler,HTTPServer
import json
import urlparse

PORT_NUMBER = 31415

# This class will handles any incoming request from the extension.
class myHandler(BaseHTTPRequestHandler):
  #Handler for the GET requests
  def do_GET(self):
    url = urlparse.urlparse(self.path)
    if (url.path != '/getmove'):
      self.send_response(404)
      self.send_header('Content-type', 'text/html')
      self.end_headers()
      self.wfile.write("File not found")
      return
    params = urlparse.parse_qs(url.query)
    print params
    request = json.loads(params['request'][0])
    print request
    self.send_response(200)
    self.send_header('Content-type', 'application/json')
    self.end_headers()
    self.wfile.write(json.dumps({
      'to': {
        'x': 3,
        'y': 3
      },
      'from': {
        'x': 3,
        'y': 1
      }
    }));
    return

try:
  # Create a web server and define the handler to manage the incoming request
  server = HTTPServer(('', PORT_NUMBER), myHandler)
  print 'Started httpserver on port ' , PORT_NUMBER
  print

  # Wait forever for incoming HTTP requests
  server.serve_forever()

except KeyboardInterrupt:
  print '^C received, shutting down the web server'
  server.socket.close()
