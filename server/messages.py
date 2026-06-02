import zmq
import sys
import os
import time
import globals
import utilities
import threading
from rich import print


class Messages:
    def __init__(self):
        self.context = zmq.Context()
        self.path = "../assets/images/temp/"
        self.socket = self.context.socket(zmq.REP)
        self.connectionDone = False
        try:
            self.socket.bind("tcp://*:5555")
        # Another instance is already using this
        except zmq.error.ZMQError:
            sys.exit(0)
        # Time out after 1 second
        self.socket.setsockopt(zmq.RCVTIMEO, 1000)
    
    def findConnection(self, lock: threading.Lock, illegalIndex = [], backup = False, first = False):
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
                # Edge case: First Connection and the server had to time out first
                if globals.hadTimeOut or response == "b\'CONNECTION\'":
                    self.socket.send_string("ROGER")
                    globals.hadTimeOut = False
                # We can finally move :)
                else:
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
                with lock:
                    self.connectionDone = True
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
            batch = utilities.getStorageData("storage.json", illegalIndex)
            self.sendYTData(batch["views"], batch["file"], batch["subs"], first)
        
        return True
    
    def sendYTData(self, views, fileNames, subCount, first = False):
        for view, file, subs in zip(views, fileNames, subCount, strict=True):
            print("views " + view)
            self.socket.send_string(view)
            status = str(self.socket.recv())
            if status == "b\'STOP\'":
                utilities.changeServerStatus("status.txt", 0)
             
            print("file " + file)
            self.socket.send_string(file)
            status = str(self.socket.recv())
            if status == "b\'STOP\'":
                utilities.changeServerStatus("status.txt", 0)
            print("subs " + subs)
            self.socket.send_string(subs)
            status = self.socket.recv()
            if status == "b\'STOP\'":
                utilities.changeServerStatus("status.txt", 0)
        
        self.socket.send_string("-1")
    
    def sendNetworkError(self):
        try:
            theResponse = str(self.socket.recv())
            self.socket.send_string("LOST")
            if theResponse == "b\'STOP\'":
                print("[red]Connection terminated")
                sys.exit(0)
        # If we get this error, frotend doesn't need videos so doesn't matter
        except zmq.error.Again:
            print("[red]stalling..")
            return
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