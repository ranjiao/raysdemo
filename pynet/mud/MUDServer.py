import socket, re, wx, time

from threading import Thread, Event

EVT_MESSAGE = wx.NewId()
EVT_NEW_CONNECTION = wx.NewId()
EVT_NEW_CLIENT = wx.NewId()
EVT_LOST_CONNECTION = wx.NewId()

class MessageEvent(wx.PyEvent):
    def __init__(self, data, type):
        wx.PyEvent.__init__(self)
        self.SetEventType(type)
        self.data = data
        
class MUDWorker(Thread):
    def __init__(self, conn, addr, window):
        Thread.__init__(self)
        self._window = window
        self._conn = conn
        self._addr = addr
        self._username = 'unknown'
        self._event_halt = Event()
        self._event_send = Event()
        self._mud_command = {
            'login': self.login, 
            'talk': self.talk
            }
        self._conn.setblocking(0)
        print('working initialized')
            
    def _event_handler(self, msg):
        wx.PostEvent(self._window, MessageEvent(msg, EVT_MESSAGE))

    def run(self):
        try:
            print('before entering main loop')
            self.main_loop()
        except Exception as e:
            self._event_handler("Connection closed or error encountered, "
                                "quiting")
            wx.PostEvent(self._window, MessageEvent(self._addr, EVT_LOST_CONNECTION))
            self._event_halt.set()
            return
            
    def stop(self):
        self._event_halt.set()
        
    def login(self, params):
        if len(params) != 2:
            self._event_handler("login command need 2 parameters, {0} is given".format(len(params)))
            return "error"

        self._username = params[0]
        wx.PostEvent(self._window, MessageEvent(str(self._username), EVT_NEW_CLIENT))
        return "login ok"
        
    def main_loop(self):
        """Conversation loop"""
        conn = self._conn
        
        while not self._event_halt.isSet():
            print('looping')
            time.sleep(0.4)
            
            if self._event_send.isSet():
                self._event_send.clear()
                print('sending message ' + self._msg)
                conn.send(self._msg)
                
            try:
                data = conn.recv(1000)
                if data is None: 
                    print('received nothing, breaking')
                    break
            except Exception as e:
                continue;
                
            print('reveiced: ' + data.decode())
            self._event_handler('Client {0}: {1}'.format(
                self._username,
                data.decode()))
                
            command, para = self.parse_command(data.decode())
            if not command: 
                self._event_handler("a invalid command received")
                continue 

            if self._mud_command.has_key(command):
                response = self._mud_command[command](para)
                conn.send(response)
            else:
                self._event_handler("Function for command {0} is not set up yet".format(command))
                conn.send(b'Server error')
        print('quiting main loop')

    def parse_command(self, cmd):
        tmp = cmd.split(' ')
        if not self._mud_command.has_key(tmp[0]):
            self._event_handler("Client submitted a wrong command {0}".format(cmd))
            self._conn.send(b'wrong command')
            return None, None
        return tmp[0], tmp[1:]

    def talk(self, params):
        self._event_handler("talk: {0}".format(' '.join(params)))
        return 'talk ok'
        
    def send(self, msg):
        self._msg = msg
        self._event_send.set()

class MUDServer(Thread):
    """Server for MUD
    """

    def __init__(self, host, port, window):
        """
        
        Arguments:
        - `host`: address for the server, optional
        - `port`: port for the server
        """
        Thread.__init__(self)
        
        self._host = host
        self._port = port
        self._state = "none"
        self._workers = []
        self._window = window
        
    def _event_handler(self, msg):
        wx.PostEvent(self._window, MessageEvent(msg, EVT_MESSAGE))

    def run(self):
        self.open()

    def open(self):
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self._socket.bind((self._host, self._port))
        self._max_recv_length = 1000
        self._max_pending_client = 5

        self._event_handler('listening on port {0}'.format(self._port))
        
        self._socket.listen(self._max_pending_client)

        conn, addr = self._socket.accept()
        
        while conn and addr:
            self._event_handler('new client connected: {0}'.format(addr))
            thread = MUDWorker(conn, addr, self._window)
            wx.PostEvent(self._window, MessageEvent(
                (addr, thread), EVT_NEW_CONNECTION))
            print('starting new thread')
            thread.start()
            self._workers.append(thread)
            
            conn, addr = self._socket.accept()

    def stop(self):
        pass

