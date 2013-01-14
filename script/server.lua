local server = {}

function accept()
  print("in accept")
end

server.start = function()
  print("server start");
  --local aid = qspawn("child", "test", {id = 2000});
  --send(aid, {id = "send"});
  qlisten(22880, "127.0.0.1", accept);
end

_G["server"] = server
