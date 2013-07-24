qnode_config = {
  -- worker config
  worker = {
    -- worker thread num
    num = 1,
  },

  -- log congfig
  log = {
    -- log file path
    path = "./log",

    -- log level
    level = "debug",
  },

  -- script config
  script = {
    -- lua script path
    path = "./qcached",
  },

  -- server config
  server = {
    daemon = 0,
  }
}
