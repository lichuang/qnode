local server = {}

server.start = function()
  print("server start");
  --c_launch("child", "test", {id = 2000});
end

_G["server"] = server
