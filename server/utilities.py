import os
import globals
import json
import random


"""Takes newly discovered views and the file locations of their
thumbnail, and writes it into a text file. Will be used in case
there are no thumbnails ready"""
def writeData(fileName, data, index):
    currentData = None
    fileData = [data.filesNames[index]]
    viewData = [data.viewCounts[index]]
    urlData = [data.viewCounts[index]]
    subData = [data.subCount[index]]
    with open(fileName, "r") as file:
        currentData = file.read()
        currentJSONData: list = json.loads(currentData)

        for location, views, url, subs in zip(fileData, viewData, urlData, subData, strict = True):
            if location not in currentData:
                if currentData:
                    currentJSONData.append({"file_name": location, "views": int(views), "subscribers": int(subs), "video_url": url})
                else:
                    currentJSONData.append({"file_name": location, "views": int(views), "subscribers": int(subs), "video_url": url})
            else:
                # Grab the previous entry
                workingEntry = [entry for entry in currentJSONData if location in entry["file_name"]][0]
                # print(oldView)
                oldFile =  workingEntry["file_name"]
                oldView = workingEntry["views"]
                oldSub =  workingEntry["subscribers"]
                oldURL = workingEntry["video_url"]
                
                # Replace everything in the string version
                currentData = currentData.replace(oldFile, location).replace(oldView, views)
                currentData = currentData.replace(oldSub, subs).replace(oldURL, url)
                
                # Reformat as JSON with the replaced items
                currentJSONData = json.loads(currentData)
                
                

    newJSONData = json.dumps(currentJSONData, indent=4)           
    with open(fileName, "w") as file:
        file.write(newJSONData)    

def randNoDupe(minInt, maxInt, used):
    randNum = random.randint(minInt, maxInt)
    while randNum in used:
        randNum = random.randint(minInt, maxInt)
    
    return randNum
    

def deleteEntry(file, views):
    text = None
    with open(file, "r") as data:
        text = data.read()
        text.replace(file + ": " + views + "\n", "")
    
    with open(file, "w") as data:
        data.write(text)

def realIndex(text: dict):


    try: 
        rIndex: str = text["file_name"]
    except IndexError:
        return -1
    rIndex = rIndex.replace(".png", "").replace("..\\assets\\images\\temp\\test", "")
    return int(rIndex)


def getStorageData(file, illegalIndexes = []):
    globals.sentIndexes.clear()
    allData = {"file": [], "views": [], "subs": []}
    jsonText = None
    
    
    with open(file, "r") as data:
        text = data.read()
            
        jsonText = json.loads(text)
        i = 0
        # Removes all the elements which are currently being used by the fethcing thread
        if illegalIndexes:
            for indexes in illegalIndexes:
                
                jsonText = [data for data in jsonText if indexes != realIndex(data)]
        
        wrapAround = False
        
        while (i < 20):
            if not jsonText:
                break
            dataIndex = random.randint(0, len(jsonText) - 1)
           
            globals.sentIndexes.append(dataIndex)
            
            sentItem = jsonText[dataIndex]
            
            # Format of the json file: file_name, views, subscribers
            allData["file"].append(sentItem["file_name"])
            # They have to sent as strings through zmq
            allData["views"].append(str(sentItem["views"]))
            allData["subs"].append(str(sentItem["subscribers"]))
            
            jsonText.pop(dataIndex)
            i += 1
   
    jsonVer = json.dumps(jsonText, indent=4)
    print(jsonVer)
   
    with open(file, "w") as data:
        data.write(jsonVer)
        
    return allData