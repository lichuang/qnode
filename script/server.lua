local server = {}

function accept()
  print("in accept")
  local connection = qnode_tcp_accept()
  print("after accept")
end

server.start = function()
  print("server start");
  --local aid = qspawn("child", "test", {id = 2000});
  --send(aid, {id = "send"});
  qnode_tcp_listen("127.0.0.1", 22880, accept);
end

_G["server"] = server
