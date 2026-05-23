import sys
import globals
import requests
import time
import psutil
import json
from rich import print
import random


"""Takes newly discovered views and the file locations of their
thumbnail, and writes it into a text file. Will be used in case
there are no thumbnails ready"""
def writeData(fileName, data, index):
    currentData = None
    fileData = [data.filesNames[index]]
    viewData = [data.viewCounts[index]]
    urlData = [data.urls[index]]
    subData = [data.subCount[index]]
    with open(fileName, "r") as file:
        currentData = file.read()
        currentJSONData: list = json.loads(currentData)

        for location, views, url, subs in zip(fileData, viewData, urlData, subData, strict = True):
            locationIndex = realIndex(location, False)
            if not globals.indexInJson[locationIndex]:
                globals.indexInJson[locationIndex] = True
                if currentData:
                    currentJSONData.append({"file_name": location, "views": int(views), "subscribers": int(subs), "video_url": url})
                else:
                    currentJSONData.append({"file_name": location, "views": int(views), "subscribers": int(subs), "video_url": url})
            else:
                print(f"[red]Location is already present in db {locationIndex}")
                # Grab the previous entry
                workingEntry = [entry for entry in currentJSONData if location in entry["file_name"]][0]
                entryIndex = currentJSONData.index(workingEntry)
                # print(oldView)
                
                newEntry = {"file_name": location, "views": int(views), "subscribers": int(subs), "video_url": url}
    
                
                # Effectively replace the old entry w/ the same img
                currentJSONData[entryIndex] = newEntry
                
                

    newJSONData = json.dumps(currentJSONData, indent=4)           
    with open(fileName, "w") as file:
        file.write(newJSONData)    

def randNoDupe(minInt, maxInt, used):
    print(f"Current List {used}")
    randNum = random.randint(minInt, maxInt)
    while randNum in used:
        print(f"Random used {randNum}")
        randNum = random.randint(minInt, maxInt)
    
    return randNum
    
"""Changes the server's status, one indicating it is 
being used, zero meaning it is not being used"""
def changeServerStatus(fileName, newStatus):
    with open(fileName, "w") as file:
        file.write(str(newStatus))

"""If the server is currently being used, it will wait
until it stops being used. Timeout specifies how much will
be waited before status is changed, maxTimeOuts makes sure
that it isn't stuck in here just in case something went wrong"""
def awaitServer(fileName: str, timeout: float, maxTimeOuts: int):
    serverUsed = True
    timeOutCount = 0
    while serverUsed:
        try:
            with open(fileName, "r") as file:
                status = int(file.read())
                if (status == 0):
                    serverUsed = False
                else:
                    globals.hadTimeOut = True
                    time.sleep(timeout)
                    timeOutCount += 1
                    if timeOutCount >= maxTimeOuts:
                        currentPrograms = psutil.process_iter()
                        instanceCount = len([p.name() for p in currentPrograms if p.name() == "server.exe"])
                        frontEnd = [p.name() for p in currentPrograms if p.name() == "more_or_less_yt.exe"]
                        # We can continue if this is the only server and the frontEnd needs us
                        if (instanceCount <= 2) and frontEnd:
                            serverUsed = False
                        # The server status got messed up somehow
                        elif (instanceCount <= 2):
                            changeServerStatus(fileName, 0)
                            sys.exit(0)
                        # The game is just running
                        else:
                            sys.exit(0)
        # If the file doesn't exist, we can just make it and ignore this step
        except FileNotFoundError:
            break
    
    changeServerStatus(fileName, 1)

def deleteEntry(file, views):
    text = None
    with open(file, "r") as data:
        text = data.read()
        text.replace(file + ": " + views + "\n", "")
    
    with open(file, "w") as data:
        data.write(text)

def realIndex(text: dict[str, str] | str, fromJson = True):


    rIndex: str = ""
    try:
        if fromJson: 
            rIndex= str(text["file_name"]) # type: ignore
        else:
            rIndex = str(text)
    except IndexError:
        return -1
    rIndex = rIndex.replace(".png", "").replace("..\\assets\\images\\temp\\test", "")
    return int(rIndex)

"""Documents whether a certain index is already being used for the
images in the JSON data. Useful for checking for duplicates"""
def documentCurrentEntries(file_name: str):
    # Guarantees list is size of MAX_CAPACITY
    for i in range(globals.IMG_CAPICITY + 1):
        globals.indexInJson.append(False)
    
    print(len(globals.indexInJson))
    with open(file_name, "r") as file:
        strData = file.read()
        
        jsonData: list = json.loads(strData)
        
        # Marks as present
        for entries in jsonData:
            print(realIndex(entries))
            globals.indexInJson[realIndex(entries)] = True

"""Pings Google to check whether the user has internet connection"""
def connectionExists():
    timeout = 1
    
    try:
        requests.head("http://www.google.com/", timeout=timeout)
        
        return True
    except requests.ConnectionError:
        
        return False

def getStorageData(file, illegalIndexes = []):
    allData = {"file": [], "views": [], "subs": []}
    jsonText = None
    
    
    with open(file, "r") as data:
        text = data.read()
            
        jsonText = json.loads(text)
        unmoddedText: list = list(jsonText)
        i = 0
        # Removes all the elements which are currently being used by the fethcing thread
        if illegalIndexes:
            for indexes in illegalIndexes:
                
                
                jsonText = [data for data in jsonText if indexes != realIndex(data)]
        
        wrapAround = False
        
        while (i < 20):
            if not jsonText:
                break

            # Choose a random video from JSON data
            sentItem = random.choice(jsonText)
            itemIndex = realIndex(sentItem)
            if (itemIndex in globals.sentIndexes): continue
            
            globals.sentIndexes.append(itemIndex)
            # It is now going to be taken away from JSON data
            globals.indexInJson[itemIndex] = False
            
            # Format of the json file: file_name, views, subscribers
            allData["file"].append(sentItem["file_name"])
            # They have to sent as strings through zmq
            allData["views"].append(str(sentItem["views"]))
            allData["subs"].append(str(sentItem["subscribers"]))
            
            # We need to delete our sent entry from both
            unmodIndex = unmoddedText.index(sentItem)
            jsonIndex = jsonText.index(sentItem)
            unmoddedText.pop(unmodIndex)
            jsonText.pop(jsonIndex)
            i += 1
    
    
    jsonVer = json.dumps(unmoddedText, indent=4)
   
    with open(file, "w") as data:
        data.write(jsonVer)
        
    return allData