import socket, wx, time
from threading import *

EVT_MESSAGE = wx.NewId()

class MessageEvent(wx.PyEvent):
    def __init__(self, data):
        wx.PyEvent.__init__(self)
        self.SetEventType(EVT_MESSAGE)
        self.data = data

class MUDClient(Thread):
    """MUD client
    """
    
    def __init__(self, window):
        """
        """
        Thread.__init__(self)
        
        self._window = window
        self._state = "none"
        self._halt = False
        self._event_send = Event()
        self._event_halt = Event()
        self._socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
        self._mud_command = {
            'login': self.login, 
            'talk': self.talk
            }
    
    def stop(self):
        self._event_halt.set()
            
    def run(self):
        self.main_loop()

    def _event_handler(self, msg):
        wx.PostEvent(self._window, MessageEvent(msg))
        
    def connect(self, host = 'localhost', port = 3742):
        print 'connecting'
        self._host = host
        self._port = port
        self._socket.connect((self._host, self._port))
        print 'connecting 2'

    def talk(self, msg):
        print('talking: {0}'.format(msg))
        self._socket.send('talk '+msg)
        #self.send('talk ' + msg)

    def login(self, username, passwd):
        socket = self._socket
        print('sending login message: ' + username + ' ' + passwd)
        username = bytes(username)
        passwd = bytes(username)
        socket.send(b'login ' + username + b' ' + passwd)
        print('waiting for login response')
        data = socket.recv(1000)
        if not data:
            self._event_handler('Cannot retrive login response from server')
            return
        if(data == b'login ok'):
            self._event_handler('logged in')
            return True
        return False
        
    def send(self, cmd):
        if cmd is None:
            return
        print('sending command {0}'.format(cmd))
        self._cmd = cmd
        self._event_send.set()
        
    def main_loop(self):
        socket = self._socket
        while not self._event_halt.isSet():
            time.sleep(0.2)
            # try receive
            try:
                data = socket.recv(1000)
                if data is None:
                    print('received nothing from server')
                self._event_handler('server: ' + data)
            except:
                pass
            finally:
                if self._event_send.isSet():
                    socket.send(bytes(self._cmd))