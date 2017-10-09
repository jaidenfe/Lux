from flask import Flask,request,jsonify,render_template
import socket,json
#from flask.ext.bcrypt import Bcrypt

#"Database"
class Data:
    uuid  = "1234"
    command = 0
    data_field = {}         #data_field is a dictionary that stores everything in the key "data", it changes constantly because it is use in communication.
    webapp_dict = {}        #Stores web client request
    server_dict = {}        #Stores server's response
    login_info= {"id":"admin@admin.com","pass":"admin"}
    user_info = {1 :{"Device":"Dummy","Group":"Dummy","serial_num":"Dummy","light_level": 0}}
    host = socket.gethostname()
    port = 8080

#This class handles what JSON string is sent to the server base on command from web client using switch statment
class JSON_S:
    def switcher(self, command):
        command_string = getattr(self, command, lambda: "nothing")
        return command_string()
    def update_req(self):
        d_string = ''
        for key in Data.data_field:
            if (key != "serial_num"):
                if isinstance(Data.data_field[key], int):
                    d_string += '"' + key + '":' +  str(Data.data_field[key]) +  ','
                else:
                    d_string += '"'+key+'":'+'"'+Data.data_field[key]+'"'+ ','
        d_string = d_string[:-1]
        c_string = '{"cmd": '+'"'+str(Data.command)+'",'+'"uuid": '+str(Data.uuid)+','+'"data":{'+d_string+'}}'
        return c_string
    def status_req(self):
        c_string = '{"cmd":' + str(Data.command) + ',"uuid":"0","serial":"' + 'SERIAL' + '"data":{' + '}}'
        return c_string
    # def status_ack(self):
    #     c_string = '{"cmd": '+ '"status_ack"}'
    #     return c_string
    # def update_ack(self):
    #     c_string = '{"cmd": '+ '"update_ack"}'
    #     return c_string

#This class handle all connection and communication from web client
class connection:
    def __init__(self,host,port,retryAttempts=3):
        self.host = host
        self.port = port
        self.retryAttempts = retryAttempts
        self.socket = None
    def connect(self, attempt = 0):
        if attempt < self.retryAttempts:
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            server_addr = (self.host, self.port)
            self.socket.connect(server_addr)
        if self.socket is None and attemps < self.retryAttempts:
            print("Connection failed"+ attempt)
            self.connect(attempt+1)
        if self.socket:
            print("Connection success! ")
    def disconnect(self):
        self.socket.close()
        self.socket = None
        print("Connection close")
    def sendToServer(self):             #The function that encode a JSON string to the server base on command
        json_s = JSON_S()
        rawMessage = json_s.switcher(Data.webapp_dict['cmd'])
        message = rawMessage.encode()
        print("sending {!r}".format(message))
        self.socket.sendall(message)
    def readServer(self):               #Reciveve JSON string and stores it in server_dict
        json_s = self.socket.recv(1024).split(b'\0', 1)[0]
        print(json_s)
        Data.server_dict = json.loads(json_s.decode('utf-8'))
        print("received {!r}".format(json_s))				#h
    def communication(self):            #The communication function control what is stores,recieve and sent commands base on web client and server
        if Data.webapp_dict['cmd'] == "status_req":         #If web client sent status_req
            Data.command = 2                                #Set command to status_req
            self.sendToServer()                             #This sents a JSON string to server base on command
            self.readServer()                               #This read from server and stores it server_dict
            Data.command = Data.server_dict['cmd']          #Stores each key individually from server_dict *might be remove since I think we can just use server dict*
            Data.uuid = Data.server_dict['uuid']
            Data.serial_num = Data.server_dict["serial"]
            Data.data_field = Data.server_dict["data"]
            Data.data_field["serial"] = Data.server_dict["serial"]
            if Data.command == 3:
                Data.user_info[2] = Data.data_field
            # Data.command = "status_ack"
            self.sendToServer()
        elif Data.webapp_dict['cmd'] == 'update_req':
            Data.command = 4
            Data.data_field = Data.webapp_dict['data']
            self.sendToServer()
            self.readServer()
            Data.command = Data.server_dict['cmd']
            if Data.command == 5:
                Data.uuid = Data.server_dict['uuid']
                Data.serial_num = Data.server_dict["serial"]
                Data.data_field = Data.server_dict["data"]
                Data.data_field["serial"] = Data.server_dict["serial"]
            # Data.command = "update_ack"
            self.sendToServer()

#Flask webframwork
app = Flask(__name__)

#Handling GET request for rendering template#
@app.route('/',methods=['GET','POST'])
def index_page():
    return render_template('index.html')

@app.route('/Luxabout.html',methods=['GET','POST'])
def about_page():
    return render_template('Luxabout.html')

@app.route('/index.html',methods=['GET','POST'])
def home_page():
    return render_template('index.html')

@app.route('/LuxLogin.html',methods=['GET','POST'])
def login_page():
    return render_template('LuxLogin.html')

@app.route('/Luxverified.html',methods=['GET','POST'])
def verified_page():
    my_json = {"cmd":"status_req"}
    Data.webapp_dict = my_json
    Data.server_dict={}
    client_py = connection(Data.host,Data.port)
    client_py.connect()
    client_py.communication()
    client_py.disconnect()
    if Data.webapp_dict['cmd'] == "status_req":
        if 2 in Data.user_info:
            return jsonify(Data.user_info)
        else:
            return jsonify({"table":"failed"})
    elif Data.webapp_dict['cmd'] == "update_req":
        print("Data: " + str(Data.data_field))
        print("Web: " + str(Data.webapp_dict))
        for keys in Data.data_field:
            if Data.data_field[keys] != Data.webapp_dict["data"][keys]:
                return jsonify({"light_level":"failed"})
        Data.user_info[2] = Data.data_field
        return jsonify(Data.user_info)

    #return render_template('Luxverified.html')

@app.route('/Luxcontact.html',methods=['GET','POST'])
def contact_page():
    return render_template('Luxcontact.html')
#Handling GET request for rendering template#

@app.route('/login',methods=['POST'])
def login():
    verification = request.get_json()
    print(verification)
    identity = verification['id']
    password = verification['pass']
    if(identity == Data.login_info["id"] and password == Data.login_info["pass"]):
        return "success"
    else:
        return "failure"

@app.route('/request',methods=['POST'])
def process():
    print("request")
    my_json = request.get_json()
    Data.webapp_dict = my_json
    Data.server_dict={}
    client_py = connection(Data.host,Data.port)
    client_py.connect()
    client_py.communication()
    client_py.disconnect()
    if Data.webapp_dict['cmd'] == "status_req":
        if 2 in Data.user_info:
            return jsonify(Data.user_info)
        else:
            return jsonify({"table":"failed"})
    elif Data.webapp_dict['cmd'] == "update_req":
        print("Data: " + str(Data.data_field))
        print("Web: " + str(Data.webapp_dict))
        for keys in Data.data_field:
            if Data.data_field[keys] != Data.webapp_dict["data"][keys]:
                return jsonify({"light_level":"failed"})
        Data.user_info[2] = Data.data_field
        return jsonify(Data.user_info)

if __name__ == "__main__":
    app.run(debug=True)
