-- worker config
worker = {
  -- worker thread num
  num = 1,
}

-- log congfig
log = {
  -- log file path
  path = "./log",

  -- log level
  level = "debug",
}

-- script config
script = {
  -- lua script path
  path = "./examples/timer",
  -- server main script
  main = "timer.lua"
}

-- server config
server = {
  daemon = 0,
  -- recycle timer internal(second)
  recycle_internal = 60,
}
