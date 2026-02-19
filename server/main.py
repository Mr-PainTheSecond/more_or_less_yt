import sqlite3
import globals
from messages import Messages
import random
import threading
import httplib2.error
import sys
from rich import print
import requests
import utilities
import html
import yt_dlp as ytd
import googleapiclient.discovery as google
import os

class YouTubeData():
    def __init__(self, firstIndex):
        self.threads: list[threading.Thread] = list()
        self.urls = []
        self.lock = threading.Lock()
        self.path = "..\\assets\\images\\temp\\"
        self.indexes = []
        self.noConnection = False

        self.filesNames = []
        self.viewCounts = []
        self.subCount = []
        self.youtube = google.build("youtube", "v3", developerKey=os.getenv("YOUTUBE_API_KEY"))

    def downloadThumbnail(self, url, count, index):
        thumnailUrl = self.getThumnbnailUrl(url, count, index)
        if not thumnailUrl:
            return
        rResponse = requests.get(thumnailUrl)
        with open(f"{self.path}test{count}.png", "wb") as file:
            file.write(rResponse.content)
            with self.lock:
                self.filesNames[index] = f"{self.path}test{count}.png"
                utilities.writeData("storage.txt", self, index)
                
        
        
    def getThumnbnailUrl(self, url, count, index):
        options = {
            "quiet": True,
            "no_warnings": True
        }
        with ytd.YoutubeDL(options) as ydl:
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
        if globals.args != "view_test":
            if category == "gaming":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 20")
            elif category == "beauty":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 26")
            elif category == "music":
                self.cursor.execute("SELECT * FROM youtube WHERE category_id = 10")
            elif category == "random":
                self.cursor.execute("SELECT * FROM youtube")
        else:
            self.cursor.execute("SELECT * FROM youtube where CHANNEL = \'jacksepticeye\'")

        
        result = self.cursor.fetchall()
        self.getFromYoutube(result)
    
    def getData(self):
        self.connection = sqlite3.connect("youtube.db")
        self.cursor = self.connection.cursor()
        a = 0
        for i in range(20):
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
            index = utilities.randNoDupe(0, 300, globals.sentIndexes)
            usedIndexes.append(index)
            globals.sentIndexes.append(index)   
            newThread = threading.Thread(None, self.downloadThumbnail, args=(self.urls[j], index, j))
            if globals.args == "view_test":
                print(self.urls[j] + " " + self.viewCounts[j])
            self.threads.append(newThread)
        
        network = threading.Thread(None, messageManager.findConnection, args = (usedIndexes, )) 
        
        for thread in self.threads:
            thread.start()
        
        if globals.args != "fill":
            network.start()
        
        for thread in self.threads:
            thread.join()
        
        if globals.args != "fill":
            network.join()


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
    global messageManager
    os.chdir("..\\server")
    firstIndex = 0
    messageManager = Messages()
    # firstBatch = utilities.getStorageData("storage.txt")
    try:
        globals.args = sys.argv[1]
    except IndexError:
        pass
    if globals.args != "fill":
        messageManager.findConnection()  
    
    repetitions = 0
    while globals.serverRunning:
        dataManager = YouTubeData(firstIndex)
        dataManager.getData()
        dataManager.getThumbnails()
        dataManager.clearAllLists()
        firstIndex = (len(dataManager.filesNames) + firstIndex) % 100
        repetitions += 1
        if globals.args == "fill" and repetitions > 2:
            break
        
       
        
        
