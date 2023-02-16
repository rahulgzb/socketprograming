Roll_Number : 21CS60R31
compilation : gcc server_salect.c -o s
             :gcc client.c -o c
than run the respective object files
Design rules
1.keep server and client at different folder in order to upload any file 
2. After entering the command at client side wait for server response 

3. I have assigned  fixed number of files that a client can upload. the fixed number can be change according to the requirement

4. I have used microsleep in upload command give sufficent time to upload file according to size. if taking more then usual time then use ctrl c to break and run the client ones again to upload.

5. Always enter the valid command at client side with valid filename 
else client will stop working for some cases .you have to use ctrl c to stop it.

6. after sending invite both client and server will be busy in taking response so 
   other client entered command will be at halt untill the client accept or reject the request.
 
 