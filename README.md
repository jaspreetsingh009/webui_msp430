Web UI for MSP430
=================

![alt tag](http://s18.postimg.org/wtonb66xl/img.png)
The web interface is created using Node.js. You will need "serialport" and "socket.io" modules to make this work. You also need to upload the launchpad code to a MSP430FR5739 launchpad (a.k.a fraunchpad)

<b>Procedure: (on ubuntu)</b>

<b>1. Install node.js: </b> <br>
&nbsp;&nbsp;&nbsp;&nbsp;sudo apt-get install python-software-properties <br>
&nbsp;&nbsp;&nbsp;&nbsp;sudo add-apt-repository ppa:chris-lea/node.js  <br>
&nbsp;&nbsp;&nbsp;&nbsp;sudo apt-get update  <br>
&nbsp;&nbsp;&nbsp;&nbsp;sudo apt-get install nodejs <br>

<b>2. Install npm (node package manager):</b> <br>
&nbsp;&nbsp;&nbsp;&nbsp;sudo apt-get install npm

<b>3. Install the aforementioned modules (first "cd" to you project directory)</b> <br>
&nbsp;&nbsp;&nbsp;&nbsp;npm install serialport  <br>
&nbsp;&nbsp;&nbsp;&nbsp;npm install socket.io  <br>

These modules will be installed in "node_modules" folder in your current directory. Now, i will recommend installing "localtunnel" which will allow you to source your localhost to a webpage (intranet to internet).

<b>4. Install localtunnel</b> <br>
&nbsp;&nbsp;&nbsp;&nbsp;sudo apt-get install ruby ruby1.8-dev rubygems1.8 libopenssl-ruby  <br>
&nbsp;&nbsp;&nbsp;&nbsp;sudo gem install localtunnel  <br>

<b>Running the server:</b>

The server will run on port 8080 (you can change this in the server.js file if required)

<b>Run server</b> &nbsp;:&nbsp;&nbsp;  node index.html  (first cd to your project directory) <br>
<b>locatunnel</b> &nbsp;&nbsp;:&nbsp;&nbsp;  lt --port 8080

The localtunnel will generate a subdomain for you and you can use that to access your website anywhere.

<b>References: <br></b>
<b>1.</b> Node beginner book : http://www.nodebeginner.org/ <br>
<b>2.</b> Understanding node.js : <a href= "http://debuggable.com/posts/understanding-node-js:4bd98440-45e4-4a9a-8ef7-0f7ecbdd56cb"> Link to the post </a> <br>
<b>3.</b> Youtube demo video:<a href ="http://www.youtube.com/watch?v=FGyU9NT8LrI"> youtube link </a>

for any queries or suggestions, feel free to contact me at jaspreetsingh009@gmail.com
