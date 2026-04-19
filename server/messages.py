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
        # Time out after 5 seconds
        self.socket.setsockopt(zmq.RCVTIMEO, 1000)
    
    def findConnection(self, illegalIndex = [], backup = False):
        # The download is complete when we got here
        with lock:
            if backup and globals.downloadComplete:
                return
        
        print("Trying to find connection...")
        print(backup)
        
        if backup:
            print("[blue]This is the backup server")
        else:
            print("[orange]This is the main server")  
        
        while True:
            try:
                response = str(self.socket.recv())
                # If a message is received, just break :)
                break
            # Designed so we can periodically check download status
            except zmq.error.Again:
                with lock:
                    if backup and globals.downloadComplete:
                        return
        
        
              
        # This is how the messages are formatted
        if response == "b\'STOP\'":
            print("[red]Connection terminated")
            if not backup:
                # Request for the backup to stop too
                self.socket.send_string("ONE_MORE")
            else:
                self.socket.send_string("DONE")
                self.socket.close() 
            globals.serverRunning = False
            sys.exit(0)
            
        with lock:
            if backup and not globals.downloadComplete:
                # Final server conversation
                self.socket.send_string("NOT_READY")
                self.socket.recv_string()
                self.socket.send_string("DONE")
                return
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