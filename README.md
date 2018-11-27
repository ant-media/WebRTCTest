# WebRTCTest

WebRTCTest Tool is a linux project for testing Ant Media Server webrtc capabilities.

* WebRTCTest is compatiple with Ant Media Server signalling protocol. 
* There is two modes of WebRTCTest: publisher and player. (-m flag determines the mode)
* You can share both camera capture or any media file with WebRTCTest. (-f flag determines the source of stream)
* WebRTCTest two options with UI or without UI. (-u flag determines the UI od/off) If you use tool in do display device you must also run `export QT_QPA_PLATFORM=offscreen` command before running the WebRTCTest.
* You can save recieved(in player mode) or published(in publisher mode) frames as png. (-p flag determines the saving periof of frames. If you set 100 it saves every 100 frames.)

It can be run from coomand promt with the following options.
```
Flag 	 Name      	 Default   	 Description                 
---- 	 ----      	 -------   	 -----------                 
s    	 Server Ip 	 localhost 	 server ip                   
q    	 Sequrity  	 false     	 true(wss) or false(ws)      
l    	 Label     	 nolabel   	 window lable                
i    	 Stream Id 	 myStream  	 id for stream               
f    	 File Name 	 camera    	 media file in same directory
m    	 Mode      	 player    	 publisher or player         
u    	 Show GUI  	 true      	 true or false               
p    	 Period    	 0         	 frame period to save as png 
v    	 Verbose   	 false     	 true or false 
```

