import subprocess
import sys
import os

os.chdir("..\\server")
try:
    import sqlite3
    import globals
    from messages import Messages
    import random
    import threading
    import time
    import httplib2.error
    import requests
    import sys
    import codecs
    from dotenv import load_dotenv
    from cryptography.fernet import Fernet
    from rich import print
    import requests
    import psutil
    import utilities
    import html
    import yt_dlp as ytd
    import googleapiclient.discovery as google
    import googleapiclient.errors as googleErrors
    
except ImportError:
    # Some requirement hasn't been made, install them
    subprocess.check_call([sys.executable, "-m", "pip", "install", "-r", "requirements.txt"])
    # We are going to start over in a new instance
    subprocess.call([sys.executable,  "main.py", "python"])
    sys.exit(0)

ytV3Doc = None

class YouTubeData():
    def __init__(self, firstIndex = 0, path = "..\\assets\\images\\temp\\", storage = "storage.json"):
        global ytV3Doc
        self.threads: list[threading.Thread] = list()
        self.urls = []
        self.lock = threading.Lock()
        self.path = path
        self.storage = storage
        self.indexes = []
        self.apiDataFile =  "youtube.v3.json"
        self.noConnection = False
        if ytV3Doc is None:
            with codecs.open(self.apiDataFile, encoding='utf-8') as file:
                ytV3Doc = file.read()

        self.noConnection = False
        self.filesNames = []
        self.viewCounts = []
        self.subCount = []
        # Have to get .env from the root folder
        os.chdir("..\\")

        # Loads our env file
        load_dotenv()
        foundAPI = False
        count = 0
        encrypted = bool(os.getenv("ENCRYPTED"))
        while not foundAPI:
            newAPI = os.getenv("YOUTUBE_API_" + str(count))
            if newAPI == "yt_key_here":
                raise Exception("Make sure to go to the .env file and set a new API key")
            
            if not newAPI:
                raise Exception("Server ran out of API keys, or the .env files does not exist")
            try:
                if encrypted:
                    # If you store the API_KEY encrypted, make sure it is its decoded (string) version
                    self.youtube = google.build_from_document(ytV3Doc, developerKey=Fernet(b'INSERT_KEY_HERE').decrypt(newAPI.encode()).decode())
                else:
                    self.youtube = google.build_from_document(ytV3Doc, developerKey=newAPI)
                    
                foundAPI = True
            except googleErrors.HttpError as e:
                # This API key is depleted
                if e.status_code in (403, 429):
                    count += 1
                else:
                    raise
                    
        os.chdir("server\\")
        

    def downloadThumbnail(self, url, count, index):
        thumnailUrl = self.getThumnbnailUrl(url, count, index)
        if not thumnailUrl:
            return
        rResponse = requests.get(thumnailUrl)
        with open(f"{self.path}test{count}.png", "wb") as file:
            file.write(rResponse.content)
            with self.lock:
                self.filesNames[index] = f"{self.path}test{count}.png"
                
        
        
    def getThumnbnailUrl(self, url, count, index):
        options = {
            "quiet": True,
            "no_warnings": True
        }
        with ytd.YoutubeDL(options) as ydl: # type: ignore
            try:
                thumbnailUrl = ydl.extract_info(url, download=False)
                return thumbnailUrl.get("thumbnail")
            except Exception:
                return None
        
    """Requests the views from YouTube and appends it to the viewCounts list
    Can either fail due to no internet or insufficient data, or succeed"""
    def appendViews(self, ytId):
        request = self.youtube.videos().list(
            part= "statistics",
            id = ytId
        )
        
        try:
            response = request.execute()
        except httplib2.error.ServerNotFoundError:
            self.noConnection = True
            globals.serverRunning = False
            print("HELLO")
            if globals.args != "fill":
                messageManager.sendNetworkError()
            print("HI")
            globals.serverRunning = False
            return False
            
        if response:
            try:
                self.viewCounts.append(response["items"][0]["statistics"]["viewCount"])
            except (KeyError, IndexError):
                return False
            self.urls.append(f"https://www.youtube.com/watch?v={ytId}")
        else:
            return False
        
        return True
    
    def appendSubs(self, channelId):
        request = self.youtube.channels().list(
            part= "statistics",
            id = channelId
        )
        
        try:
            response = request.execute()
        except httplib2.error.ServerNotFoundError:
            self.noConnection = True
            print("HELLO")
            if globals.args != "fill":
                messageManager.sendNetworkError()
            print("HI")
            globals.serverRunning = False
            return False
            
        if response:
            try:
                self.subCount.append(response["items"][0]["statistics"]["subscriberCount"])
            except KeyError:
                return False
        else:
            return False
        
        return True
    
    def getFromYoutube(self, result):
        videoRequested = random.choice(result)
        # Index 2 is always channelId, 4 is always ytId (it is how the database is organized)
        ytId = videoRequested[4]
        channelId = videoRequested[2]
        
        # We only add everything else if this is successful
        if not self.appendViews(ytId):
            return
        
        if not self.appendSubs(channelId):
            # If our data is incomplete, we are not adding anything
            self.viewCounts.pop()
            self.urls.pop()
            return
        

    def handleCategory(self):
        
        category = random.choice(globals.typeList)
        result = None
        # Forces videos from Jacksepticeye so we can check if views are good
        if globals.args == "view_test":
            self.cursor.execute("SELECT * FROM youtube where CHANNEL = \'jacksepticeye\'")
        
        else:
            if category == "gaming":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 20")
            elif category == "beauty":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 26")
            elif category == "music":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 10")
            elif category == "pets":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 15")
            elif category == "sports":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 17")
            elif category == "politics":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 25")
            elif category == "science":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 28")
            elif category == "random":
                self.cursor.execute("SELECT * FROM youtube")
        

        
        result = self.cursor.fetchall()
        self.getFromYoutube(result)
    
    def getData(self, count = 30):
        a = 0
        
        # Database stuff
        self.connection = sqlite3.connect("youtube.db")
        self.cursor = self.connection.cursor()
        
        for i in range(count):
            self.handleCategory()
            if globals.args == "view_test":
                try:
                    a += 1
                except:
                    continue
        
    
    def getThumbnails(self):
        usedIndexes = []
        print(f"{len(self.urls)}, {len(self.viewCounts)}")
        for j in range(len(self.urls)):
            # Force the file names to be the size we want it to be
            self.filesNames.append(-1)
            index = utilities.randNoDupe(0, globals.IMG_CAPICITY, globals.sentIndexes)
            usedIndexes.append(index)
            globals.sentIndexes.append(index)   
            newThread = threading.Thread(None, self.downloadThumbnail, args=(self.urls[j], index, j))
            if globals.args == "view_test":
                print(self.urls[j] + " " + self.viewCounts[j])
            self.threads.append(newThread)
        
        if not utilities.connectionExists():
            utilities.changeServerStatus("status.txt", 0)
            sys.exit(0)
        
        network = threading.Thread(None, messageManager.findConnection, args = (self.lock, usedIndexes, ))
        backupNetwork = threading.Thread(None, messageManager.findConnection, args = (self.lock, usedIndexes, True,  )) 
        
       
        globals.downloadComplete = False
        
        for thread in self.threads:
            thread.start()
        
        # Starts the main server
        if globals.args != "fill" and not messageManager.connectionDone:
            network.start()
            
        if globals.args != "fill" and not messageManager.connectionDone:
            network.join()    
        
        print("Main Network has finished")
        
        # Once the main server is done, we will do backup
        if globals.args != "fill" and not messageManager.connectionDone:
            backupNetwork.start() 
        
        for thread in self.threads:
            thread.join()
        
        # This will make backup quit next iteration
        with self.lock:
            globals.downloadComplete = True
        
        if globals.args != "fill" and not messageManager.connectionDone:
            backupNetwork.join()     
        

        for index in range(len(self.urls) - 1):
            # Flags indicating couldn't download video
            if self.filesNames[index] != -1:
                utilities.writeData(self.storage, self, index)
        
        globals.sentIndexes.clear()
        
        print("[green]Cycle complete")
        for file in self.filesNames:
            if file == -1:
                indexes = self.filesNames.index(file)
                self.filesNames.pop(indexes)
                self.urls.pop(indexes)
                self.viewCounts.pop(indexes)
        
        
        a = 0
        # for view, url, file in zip(self.viewCounts, self.urls, self.filesNames,strict=True):
        #     print(a)
        #     print(file)
        #     print(view + " " + url + " ")
        #     a += 1

    
    
    def cleanData(self):
        for image in self.filesNames:
            os.remove(image)
    
    """After a cycle is complete, it is important that all the data
    from the previous iteration are all cleared"""
    def clearAllLists(self):
        self.filesNames.clear()
        self.viewCounts.clear()
        self.urls.clear()
        self.subCount.clear()
        
if __name__ == "__main__":
    try:
        globals.args = sys.argv[1]
        if globals.args not in globals.debugModes:
            try:
                os.chdir(globals.args)
                os.chdir("..\\server")
            except Exception as e:
                print(e)
                time.sleep(20)
                raise
                
    except IndexError:
        pass
    
    # Checks if the server is already running
    utilities.awaitServer("status.txt", 5, 4)
    
    
    global messageManager
    firstIndex = 0
    
    utilities.documentCurrentEntries("storage.json")
    messageManager = Messages()
    # firstBatch = utilities.getStorageData("storage.txt")
    
    # This lock is used for the first iteration only
    firstLock = threading.Lock()
    # For running in debug modes
    if globals.args != "fill":
        
         
        
        if not utilities.connectionExists():
            messageManager.sendNetworkError()
        # We can start sending data if internet connection
        else:
            
            messageManager.findConnection(firstLock, first=True)
            
    
    repetitions = 0
    while globals.serverRunning:
        
        dataManager = YouTubeData(firstIndex)
        if not utilities.connectionExists():
            messageManager.sendNetworkError()
            continue
         # Data Collection takes time!!
        serverBuffer = threading.Thread(None, messageManager.findConnection, args = (dataManager.lock, [], True,  ))
        dataCollector = threading.Thread(None, dataManager.getData)
        
        globals.downloadComplete = False
        
        if globals.args != "fill":
            serverBuffer.start()
        
        dataCollector.start()
        
        
        dataCollector.join()
        
        with dataManager.lock:
            globals.downloadComplete = True
        
        if globals.args != "fill":
            serverBuffer.join()
        
        dataManager.getThumbnails()
        dataManager.clearAllLists()
        firstIndex = (len(dataManager.filesNames) + firstIndex) % 100
        repetitions += 1
        if globals.args == "fill" and repetitions > 2:
            break
    
    utilities.changeServerStatus("status.txt", 0)
        
       
        
        
