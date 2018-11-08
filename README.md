# WebRTCTest
Linux WebRTC Test Tool project for Ant Media Server

This tool is compatiple with Ant Media Server signalling protocol. You can share both camera capture or any media file with this WebRTCTest tool.

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
