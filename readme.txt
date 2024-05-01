a. Your Full Name as given in the class list
Zeqing Wang 
b. Your Student ID
1544834203
c. What you have done in the assignment, if you have completed the optional part (extra 
credit). If it’s not mentioned, it will not be considered.
I finished the required part of project,I also finish a XOR method for extra encryption, and apply it to both username and password.
for extra:
make clean 
make extra
./serverM ./serverS ./serverD ./serverU ./client
and the program will run automaticly
the extraencrypted data is stored in member_extra.txt, and please use the username and password in original member_unencrypted.txt for test input
d. What your code files are and what each one of them does. (Please do not repeat the 
project description, just name your code files and briefly mention what they do).
serverM.cpp handle the client request and communicate with backend server
serverS/D/U.cpp load the room data and reply for the serverM query
client.cpp the front end for client, help client to talk to server<
e. The format of all the messages exchanged, e.g., username and password are concatenated 
and delimited by a comma, etc.
the data are divided in ","
f. Any idiosyncrasy of your project. It should say under what conditions the project fails, if 
any.
N/A, but if the its shows address already occupoed, please close all the command and make clean and retest it. 
g. Reused Code: Did you use code from anywhere for your project? If not, say so. If so, say 
what functions and where they're from. (Also identify this with a comment in the source 
code). Reusing functions which are directly obtained from a source on the internet 
without or with few modifications is considered plagiarism (Except code from the Beej’s 
Guide). Whenever you are referring to an online resource, make sure to only look at the 
source, understand it, close it and then write the code by yourself. The TAs will perform 
plagiarism checks on your code so make sure to follow this step rigorously for every 
piece of code which will be submitted.
I use some code from Beej's tutorial, and cite in files. 