from flask import Flask, request, jsonify, render_template
import socket, json

sock = None#socket
host = "127.0.0.1"#host address
port = 8080
buf_size = 1024#max message size
dict = None#last recieved message in map format
connected = False

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

        resp = '{"1" :' + read().decode('utf-8') + "}"#TODO read and return combined JSON once end delimiter/message is found (not yet implemented by server)

        print(resp)
    #render_template('dashboard.html')
    return resp


@app.route('/update_req', methods=['POST'])
def update_req():
    rcvd = request.get_json()
    if (rcvd != None):
        command = rcvd["cmd"]
        uuid = rcvd["uuid"]
        serial = rcvd["serial"]
        name = rcvd["data"]["name"]
        level = rcvd["data"]["level"]

        msg = '{"cmd":' + str(command) + ',"uuid":"' + uuid + '","serial":"' + serial + '","data":{"name":"' + name + '","level":"' + str(level) + '"}}'
        send(msg)

        resp = read().decode('utf-8')
    return resp#render_template('dashboard.html')

@app.route('/unregister', methods=['POST'])
def unregister():#TODO
    rcvd = request.get_json()
    if (rcvd != None):
        command = UNREGISTER
        uuid = "0"
        serial = rcvd["serial"]
        msg = '{"cmd":' + str(command) + ',"uuid": uuid ,"serial": serial ,"data":{}}'
        connect()
        send(msg)
        disconnect()
    return render_template('dashboard.html')

def connect():
    global connected, sock
    if (connected):
        return#don't make a second connection
    sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    sock.setblocking(True)
    sock.connect((host, port))
    connected = True
    print("Connection established.")

def disconnect():
    global connected, sock
    if (not connected):
        return#not connected, don't try to d/c

    msg = '{"cmd":"7","uuid":"0","serial":"0","data":{}}'
    send(msg)#DISCONNECT -> client_exit();

    sock.close()
    sock = None
    connected = False
    print("Connection terminated.")

def send(msg):
    global connected, sock
    if (not connected or sock == None):
        print("Attempted to send data without connection.")
        return
    sock.sendall(str.encode(msg))

def read():
    msg = sock.recv(buf_size).split(b'\0', 1)[0]
    return msg
    #dict = json.loads(msg.decode("utf-8"))

if __name__ == "__main__":
    app.run(host='0.0.0.0')
