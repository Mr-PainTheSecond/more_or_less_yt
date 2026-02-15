import zmq
import sys
import os
import globals
import utilities
import threading
from rich import print

lock = threading.Lock()

class Messages:
    def __init__(self):
        self.context = zmq.Context()
        self.path = "../assets/images/temp/"
        self.socket = self.context.socket(zmq.REP)
        self.socket.bind("tcp://*:5555")
    
    def findConnection(self, illegalIndex = []):
        print("Trying to find connection...")
        response = str(self.socket.recv())
        # This is how the messages are formatted
        if response == "b\'STOP\'":
            print("[red]Connection terminated")
            self.socket.close()
            globals.serverRunning = False
            sys.exit(0)
        print("[green]Established a connection")
        with lock:
            batch = utilities.getStorageData("storage.txt", illegalIndex)
            self.sendYTData(batch["views"], batch["file"], batch["subs"])
        
        return True
    
    def sendYTData(self, views, fileNames, subCount):
        for view, file, subs in zip(views, fileNames, subCount, strict=True):
            self.socket.send_string(view)
            self.socket.recv()
            self.socket.send_string(file)
            self.socket.recv()
            self.socket.send_string(subs)
            self.socket.recv()
        
        self.socket.send_string("-1")
    
    def sendNetworkError(self):
        self.socket.recv()
        self.socket.send_string("LOST")
    # def talkToClient(self, *args):
    #     files = list(args)
    #     while files:
    #         response = self.socket.recv()
    #         if "Delete" in str(response):
    #             os.remove(files[0])
    #             files.pop(0)
    #             self.socket.send_string("roger")
    #         elif "Terminate" in str(response):
    #             self.socket.close()
    #             self.socket = self.context.socket(zmq.REP)
    #             self.socket.bind("tcp://*:5555")
    #             return