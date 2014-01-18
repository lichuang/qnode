require("util")

local server = {}

server.start = function()
  qlog("timer start");

  qlog("before timer");
  qlsleep(1);
  qlog("after timer");
end

_G["server"] = server
