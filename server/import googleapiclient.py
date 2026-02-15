import googleapiclient.discovery as google
import requests
import yt_dlp as ytd
import os

url = "https://www.youtube.com/watch?v=klfBwzFRCuo"
fileName = "zack.JPG"

os.chdir("C:\\Users\\santi\\source\\repos\\more_or_less_yt\\assets\\images\\perm\\thumbnail")
options = {
    "quiet": True,
    "no_warnings": True
}

response = None
with ytd.YoutubeDL(options) as ydl:
    thumbnailUrl = ydl.extract_info(url, download=False)
    data = thumbnailUrl.get("thumbnail")
    print(data)
    response = requests.get(data)

with open(fileName, "wb") as file:
    file.write(response.content)
    