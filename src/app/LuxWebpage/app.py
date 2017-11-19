from flask import Flask, request, jsonify, render_template
from flask_ask import Ask, statement, question, session
import socket, json, logging, threading, time

sock = None#socket
host = "127.0.0.1"#host address
port = 8080
buf_size = 1024#max message size
dict = None#last recieved message in map format
dev_connected = {}
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
ask = Ask(app, '/')
app.secret_key = 'secret'
app.config['ASK_VERIFY_REQUESTS'] = False
# logging.getLogger("flask_ask").setLevel(logging.DEBUG)

#time.sleep(10)

def send_msg_60():
    global dev_connected
    threading.Timer(60.0, send_msg_60).start()
    connect()
    if(connected ==  False):
        return
    command = STATUS_REQUEST
    msg = '{"cmd":' + str(command) + ',"uuid":"0","serial":"0","data":{}}'
    send(msg)#status_req
    deviceDict={}
    while (True):
        rmsg = read().decode()
        # print(rmsg)
        if (rmsg == status_req_delim):
            break
        jsonMSG = json.loads(rmsg)
        # print(jsonMSG)
        deviceDict[jsonMSG["data"].get("name")] = jsonMSG.get("serial")
    dev_connected = deviceDict
    print(dev_connected)

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
    #send_msg_10()
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
            rmsg = read().decode('utf-8')
            if (rmsg == status_req_delim):
                break
            if (count > 1):
                resp = resp + ","
            resp = resp + '"' + str(count) + '" :' + rmsg
            count = count + 1
        resp = resp + "}"
    # resp = {1:{"serial": "xg9gt5hd7651", "data": {"group_name": "Life", "name": "Lux", "level": 10}},2: {"serial": "1567hd5tg9gx", "data": {"group_name": "Baby", "name": "LuxZ", "level": 0}},3: {"serial": "1567hx", "data": {"group_name": "Life", "name": "LuxB", "level": 0}}}
    print(resp)
    #render_template('dashboard.html')
    return resp


@app.route('/update_req', methods=['POST'])
def update_req():
    rcvd = request.get_json()
    # print(rcvd)
    if (rcvd != None):
        command = rcvd["cmd"]
        uuid = rcvd["uuid"]
        serial = rcvd["serial"]
        name = rcvd["data"]["name"]
        level = rcvd["data"]["level"]
        group = rcvd["data"]["group_name"]

        msg = '{"cmd":' + str(command) + ',"uuid":"' + uuid + '","serial":"' + serial + '","data":{"name":"' + name + '","level":"' + str(level) + '","group_name":"' + group + '"}}'
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
    try:
        sock.connect((host, port))
    except Exception as e:
        print("connection failed")
        connected  = False
        return
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

@ask.launch
def start_skill():
    connect()
    welcome_msg = "Hello, welcome to Lux! If you need help, say help"
    RPmsg = "Session will end in 8 secs.... If you need help say help"
    print(welcome_msg)
    return question(welcome_msg).reprompt(RPmsg)

@ask.intent("OnIntent")
def turn_on_skill(device):
    alexa_msg = device
    device = device.upper()
    device = device.replace(" ","_")
    if device in dev_connected:
        # devName = device.replace(" ","_")
        server_msg = '{"cmd":4,"uuid":"0","serial":"' + dev_connected[device] + '","data":{"name":"'+device+'","level":"10","group_name":"all"}}'
        print(server_msg)
        send(server_msg)
        msg = alexa_msg + " is ON"
        rcvd = read()
    else:
        msg = "Sorry I don't see the device you specify, please try agin."
    RPmsg = "Session will end in 8 secs.... If you need help say help"
    print(msg)
    return question(msg).reprompt(RPmsg)

@ask.intent("OffIntent")
def turn_off_skill(device):
    alexa_msg = device
    device = device.upper()
    device = device.replace(" ","_")
    if device in dev_connected:
        # devName = device.replace(" ","_")
        server_msg = '{"cmd":4,"uuid":"0","serial":"' + dev_connected[device] + '","data":{"name":"'+device+'","level":"10","group_name":"all"}}'
        print(server_msg)
        send(server_msg)
        msg = alexa_msg + " is OFF"
        rcvd = read()
    else:
        msg = "Sorry I don't see the device you specify, please try agin."
    RPmsg = "Session will end in 8 secs.... If you need help say help"
    print(msg)
    return question(msg).reprompt(RPmsg)

@ask.intent("helpIntent")
def help_command():
    msg = "To turn on device, say turn on ... device name. To turn off device, say turn off ... device name."
    RPmsg = "Session will end in 8 secs.... If you need help say help"
    print(msg)
    return question(msg).reprompt(RPmsg)

@ask.intent("OnGroupIntent")
def turn_on_group_skill(group):
    if(group == "all"):
        for device in dev_connected:
            server_msg = '{"cmd":4,"uuid":"0","serial":"' + dev_connected[device] + '","data":{"name":"'+device+'","level":"10","group_name":"all"}}'
            print(server_msg)
            send(server_msg)
            rcvd = read()
        msg = "Group " + group + " is ON"
    else:
        msg = "Sorry I don't see the group you specify, please try agin."
    RPmsg = "Session will end in 8 secs.... If you need help say help"
    print(msg)
    return question(msg).reprompt(RPmsg)

@ask.intent("OffGroupIntent")
def turn_off_group_skill(group):
    if(group == "all"):
        for device in dev_connected:
            server_msg = '{"cmd":4,"uuid":"0","serial":"' + dev_connected[device] + '","data":{"name":"'+device+'","level":"0","group_name":"all"}}'
            print(server_msg)
            send(server_msg)
            rcvd = read()
        msg = "Group " + group + " is OFF"
    else:
        msg = "Sorry I don't see the group you specify, please try agin."
    RPmsg = "Session will end in 8 secs. If you need help say help"
    print(msg)
    return question(msg).reprompt(RPmsg)

@ask.session_ended
def session_ended():
    return "{}",200

send_msg_60()

if __name__ == "__main__":
    app.run(host='0.0.0.0', port=80)
    threaded = True

