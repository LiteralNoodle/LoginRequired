from http.server import HTTPServer, BaseHTTPRequestHandler

SERVER_IP = 'localhost'
SERVER_PORT = 8080
ACCOUNTS_FILE = 'users'

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

        # default path
        if url_dir == '/' or url_dir == '':
            url_dir = '/index.html'

        # try to open the requested page
        try:
            file_contents = open(url_dir[1:]).read()
            self.send_response(200)

        except:
            file_contents = "File not found"
            self.send_response(404)

        # handle account creation if required
        if url_dir == '/createaccount':
            if parsed_query_parameters != None:
                if parsed_query_parameters['username'] != None and parsed_query_parameters['hash'] != None:
                    user_file = open(ACCOUNTS_FILE, 'r+')
                    user_dict = self.read_users(user_file)
                    if parsed_query_parameters['username'] not in user_dict.keys():
                        user_file.write(f'{parsed_query_parameters["username"]} {parsed_query_parameters["hash"]}\n')
                        file_contents = 'User account successfully created!'
                    else:
                        file_contents = 'That user account already exists! Account not created.'
                    user_file.close()

        # send the response (requested page or otherwise)
        self.end_headers()
        self.wfile.write(bytes(file_contents, 'utf-8'))

    # for parsing the user file and checking if user present 
    def read_users(self, user_file):
        users = {}
        for line in user_file.read().split('\n'):
            if line != '':
                username = line.split(' ')[0]
                hash = line.split(' ')[1]
                users[username] = hash

        return users


if __name__ == "__main__":
    httpd = HTTPServer((SERVER_IP, SERVER_PORT), Serv)
    print("Server is ready...")
    httpd.serve_forever()