from flask import Flask, request, jsonify, render_template
import socket, json

sock = None#socket
host = "127.0.0.1"#host address
port = 8080
buf_size = 1024#max message size
dict = None#last recieved message in map format
connected = False
status_req_delim = "STOP"

REGISTER = 0
CONNECT = 1
STATUS_REQUEST = 2
STATUS = 3
UPDATE_REQUEST = 4
UPDATE = 5
DISCONNECT_REQUEST = 6
DISCONNECT = 7
UNREGISTER = 8
FORCE_DISCONNECT = 9
TEST = 10
REG_REQUEST = 11

app = Flask(__name__)

@app.route('/', methods=['GET'])
@app.route('/index.html', methods=['GET'])
def index():
    return render_template('index.html')

@app.route('/about.html', methods=['GET'])
def about():
    return render_template('about.html')

@app.route('/dashboard.html', methods=['GET'])
def dashboard():
    connect()
    return render_template('dashboard.html')

@app.route('/status_req', methods=['POST'])
def status_req():
    rcvd = request.get_json()
    resp = ""
    if (rcvd != None):
        command = STATUS_REQUEST
        msg = '{"cmd":' + str(command) + ',"uuid":"0","serial":"0","data":{}}'
        send(msg)#status_req
		
        count = 1;
        resp = "{"
        
        while (True):
            rd = read()
            if (rd == b''):
                break#ERROR
            rmsg = rd.decode('utf-8')
            if (rmsg == status_req_delim):
                break
            if (count > 1):
                resp = resp + ","
            resp = resp + '"' + str(count) + '" :' + rmsg
            count = count + 1
        resp = resp + "}"
        
        print(resp)
    #render_template('dashboard.html')
    return resp
        

@app.route('/update_req', methods=['POST'])
def update_req():
    rcvd = request.get_json()
    if (rcvd != None):
        try:
            command = rcvd["cmd"]
            uuid = rcvd["uuid"]
            serial = rcvd["serial"]
            name = rcvd["data"]["name"]
            level = rcvd["data"]["level"]
            group = rcvd["data"]["group_name"]
        
            msg = '{"cmd":' + str(command) + ',"uuid":"' + uuid + '","serial":"' + serial + '","data":{"name":"' + name + '","level":"' + str(level) + '","group_name":"' + group + '"}}'
            send(msg)
        
            resp = read().decode('utf-8')
        except:
            print("ERROR: Improper JSON recieved in update request command.")
    return resp#render_template('dashboard.html')

@app.route('/unregister', methods=['POST'])
def unregister():#TODO
    rcvd = request.get_json()
    if (rcvd != None):
        try:
            command = UNREGISTER
            uuid = "0"
            serial = rcvd["serial"]
            msg = '{"cmd":' + str(command) + ',"uuid": uuid ,"serial": serial ,"data":{}}'
            connect()
            send(msg)
            disconnect()
        except:
            print("ERROR: Improper JSON recieved in unregister command.")
    return render_template('dashboard.html')

def connect():
    global connected, sock
    if (connected):
        return#don't make a second connection
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.setblocking(True)
        sock.connect((host, port))
        connected = True
        print("Connection established.")
    except:
        print("ERROR: Connection to server failed!")

def disconnect():
    global connected, sock
    if (not connected):
        return#not connected, don't try to d/c
    
    try:
        msg = '{"cmd":"7","uuid":"0","serial":"0","data":{}}'
        send(msg)#DISCONNECT -> client_exit();
    
        sock.close()
        sock = None
        connected = False
        print("Connection terminated.")
    except:
        print("ERROR: Failed to disconnect from server!")

def send(msg):
    global connected, sock
    if (not connected or sock == None):
        print("ERROR: Attempted to send data without connection.")
        return
    try:
        sock.sendall(str.encode(msg))
    except:
        print('ERROR: Failed to send message - "' + msg + '" - to the server.')
        disconnect()

def read():
    try:
        msg = sock.recv(buf_size).split(b'\0', 1)[0]
        return msg
    except:
        print("ERROR: Failed to recieve message from the server.")
        disconnect()
    return b''
    #dict = json.loads(msg.decode("utf-8"))

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=80)
