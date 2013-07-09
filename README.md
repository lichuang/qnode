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
    		path = "./script", -- lua script path
  		},

  		-- server config
  		server = {
    		daemon = 0, -- if the server run in daemon mode
  		}
	}

- Lua scripts organization




  


