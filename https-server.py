#!/usr/bin/env python2


# taken from http://www.piware.de/2011/01/creating-an-https-server-in-python/
# generate server.xml with the following command:
#    openssl req -new -x509 -keyout server.pem -out server.pem -days 365 -nodes
# run as follows:
#    python simple-https-server.py
# then in your browser, visit:
#    https://localhost:4443


import BaseHTTPServer
import SimpleHTTPServer
import SocketServer
import logging
import ssl


PORT = 8000


class GetHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):

    def do_GET(self):
        logging.error(self.headers)
        SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)


Handler = GetHandler
httpd = BaseHTTPServer.HTTPServer(('0.0.0.0', 4443), Handler)
httpd.socket = ssl.wrap_socket(
    httpd.socket, certfile='./https-server.pem', server_side=True)
httpd.serve_forever()
