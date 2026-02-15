import os
import globals
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
        entrySeperate = currentData.split("\n")
 
        
        
        for location, views, url, subs in zip(fileData, viewData, urlData, subData, strict = True):
            if location not in currentData:
                if currentData:
                    currentData += f"\n{location}: {views}: {url}: {subs}"
                else:
                    currentData += f"{location}: {views}: {url}: {subs}"
            else:
                workingEntry = [entry for entry in entrySeperate if location in entry]
                # print(oldView)
                oldView = workingEntry[0].split(":")[1]
                oldUrl =  workingEntry[0].split(":")[2]
                oldSub =  workingEntry[0].split(":")[3]
                currentData = currentData.replace(": " + oldView, ": " + views)
                currentData = currentData.replace(": " + oldUrl, ": " + url)
                currentData = currentData.replace(": " + oldSub, ": " + subs)
                
            entrySeperate = currentData.split("\n")

               
        
    with open(fileName, "w") as file:
        file.write(currentData)    

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

def realIndex(text: str):


    try: 
        rIndex: str = text.split(":")[0]
    except IndexError:
        return -1
    rIndex = rIndex.replace(".png", "").replace("..\\assets\\images\\temp\\test", "")
    return int(rIndex)


def getStorageData(file, illegalIndexes = []):
    globals.sentIndexes.clear()
    allData = {"file": [], "views": [], "subs": []}
    text = None
    
    with open(file, "r") as data:
        text = data.read()
        
        with open("test_pre.txt", "w") as test:
            test.write(text)
            
        splitText = text.split("\n")
        i = 0
        # Removes all the elements which are currently being used by the fethcing thread
        if illegalIndexes:
            for indexes in illegalIndexes:
                
                splitText = [split for split in splitText if indexes != realIndex(split)]
        
        wrapAround = False
        
        while (i < 20):
            if not splitText:
                break
            randomElement = random.choice(splitText)
            globals.sentIndexes.append(randomElement)
            # if (rIndex >= illegalIndex and rIndex <= illegalIndex):
            #     if wrapAround:
            #         break
                    
            #     wrapAround = True
            #     index = (illegalIndex + 20) % 120
            #     rIndex = realIndex(text, index)
            #     continue
            # try:
            #     newData = splitText[index]
            # except IndexError:
            #     index += 1
            #     rIndex = realIndex(text, index)
            #     if (rIndex > 120):
            #         index = 0
            #         rIndex = realIndex(data, index)
            #     continue
                
            splitData = randomElement.split(":")
            
            # Removes spacing before it is sent
            splitData[1] = splitData[1].replace(" ", "")
            splitData[3] = splitData[3].replace(" ", "")
            # Formatted as File: Views in txt file
            allData["file"].append(splitData[0])
            allData["views"].append(splitData[1])
            allData["subs"].append(splitData[3])
            # Removes what we used
            text = text.replace(randomElement + "\n", "")
            splitText.remove(randomElement)
            
            i += 1
            # index += 1
            # if (index > 100):
            #     index = 0
            # rIndex = realIndex(text, index)
        
   
    with open(file, "w") as data:
        data.write(text)
    return allData