from http.server import HTTPServer, BaseHTTPRequestHandler
from ast import literal_eval

SERVER_IP = 'localhost'
SERVER_PORT = 8080
ACCOUNTS_FILE = 'users'

class Serv(BaseHTTPRequestHandler):

    def do_GET(self):

        print(self.path)
        url_dir = self.path

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

        # send the response (requested page or otherwise)
        self.end_headers()
        self.wfile.write(bytes(file_contents, 'utf-8'))

    # POST for when making a new account or retrieving an account
    def do_POST(self):

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

        # handle login
        if url_dir == "/trylogin":
            # get request body as dictionary 
            content_length = int(self.headers.get('Content-Length'))
            post_body = self.rfile.read(content_length).decode('utf-8')
            
            # check against existing accounts
            try:
                user_file = open(ACCOUNTS_FILE, 'r+')
                user_dict = self.read_users(user_file)
                post_body = literal_eval(post_body) # convert str to dict

                if post_body['username'] in list(user_dict.keys()):
                    print(f'Found user: {post_body["username"]}')
                    # check if hash matches 
                    if post_body['passwordHash'] == user_dict[post_body['username']]:
                        # success. login. 
                        print(f'Logged in {post_body["username"]}')
                        self.send_response(301)
                        self.send_header('Location', '/logged_in.html')
                        self.end_headers()
                    else:
                        # fail. back to login.
                        print(f"Failed to login {post_body['username']}")
                        self.send_response(301)
                        self.send_header('Location', '/login_failed.html')
                        self.end_headers()
                else:
                    print(f"Failed to login {post_body['username']}")
                    self.send_response(301)
                    self.send_header('Location', '/login_failed.html')
                    self.end_headers()

                user_file.close()

            except:
                print("Error checking account file.")

        file_contents = self.get_requested_content(url_dir)

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

    # func for getting requested file
    def get_requested_content(self, url_dir):
        # try to open the requested page
        try:
            file_contents = open(url_dir[1:]).read()
            self.send_response(200)

        except:
            file_contents = "File not found"
            self.send_response(404)

        return file_contents

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