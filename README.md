## README for qnode ##

### What is qnode ###
  qnode(cute node) - C + Lua + Actor Model(implement in Lua coroutine) = Erlang-like system

### Compile  
  qnode is implemented in pure ANSI C, and compile in Linux(which support epoll).

### How to use ###
  When qnode start,it will read a config file, you can use -c option to assign a config file.By default,it will use qnodepath/etc/config.lua.

- Config file format
  
  qnode config file is a Lua script.An example config file is in qnodepath/etc/config.lua:
 

		qnode_config = {  
		-- worker config  
		worker = {
      		num = 1, -- worker thread num
		},

		-- log congfig
		log = {
    		path = "./log",	 -- log file path
    		level = "debug", -- log level
  		},

  		-- script config
  		script = {
    		path = "./qcached", -- lua script path
  		},

  		-- server config
  		server = {
    		daemon = 0, -- if the server run in daemon mode
  		}
	}

- Lua scripts organization

  When qnode start, it reads Lua script files which locate in config file script/path.First,it reads a main.lua file, which load all Lua files needed.It looks like this:

		package.path = "./?.lua;../script/?.lua"

		require("util")

		require_ex("server")
		require_ex("child")
  In the first line, package.path assign where lua files located, in path "./script" there is a lua script util.lua which provide some util functions such as qlog and require_ex.

  After load and execute main.lua, qnode start to execute a function named server.start:

		server.start = function()
	  		qlog("server start");
	
	  		-- spawn storage process
	  		local storage_id = qnode_spawn("server", "storage")
	
	  		-- accept connection
	  		local socket = qtcp_listen(22880);
	  		accept(socket, storage_id)
		end
	
		_G["server"] = server

  In server.start function,you can do whatever what a server start,such as listening a socket,spawn a qnode actor.

- Demo

  In qnode_path/qcached,there is a simple server which write in lua using qnode API,it support simple memcache get/set protocol.




  


