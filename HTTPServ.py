from http.server import HTTPServer, BaseHTTPRequestHandler

class Serv(BaseHTTPRequestHandler):

    def do_GET(self):

        print(self.path)

        # split into the directory path and the query parameters 
        split_url = self.path.split('?')
        url_dir = split_url[0]
        if len(split_url) == 2:
            url_query_section = self.path.split('?')[1]

            # get all of the key-value pairs and throw them into a dict
            query_pairs = url_query_section.split('&')
            parsed_query_parameters = {}
            for p in query_pairs:
                key = p.split('=')[0]
                value = p.split('=')[1]
                parsed_query_parameters[key] = value

            print(parsed_query_parameters)

        if url_dir == '/' or url_dir == '':
            url_dir = '/index.html'

        try:
            file_contents = open(url_dir[1:]).read()
            self.send_response(200)

        except:
            file_contents = "File not found"
            self.send_response(404)

        self.end_headers()
        self.wfile.write(bytes(file_contents, 'utf-8'))


httpd = HTTPServer(('localhost', 8080), Serv)
print("Server is ready...")
httpd.serve_forever()