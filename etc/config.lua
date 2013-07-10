qnode_config = {
  -- worker config
  worker = {
    num = 1,  -- worker thread num
  },

  -- log congfig
  log = {
    path = "./log",   -- log file path
    level = "debug",  -- log level
  },

  -- script config
  script = {
    path = "./qcached",-- lua script path
  },

  -- server config
  server = {
    daemon = 0,       -- lua script path
  }
}
