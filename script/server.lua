local server = {}

server.child = function (_sock)
  print("in child: id " .. _sock["id"])
  qnode_tcp_recv(_sock["sock"])
  print("after child")
end

function accept()
  print("in accept")
  local socket = qnode_tcp_accept()
  local aid = qnode_spawn("server", "child", {id = 200, sock = socket})
  print("after accept")
end

server.start = function()
  print("server start");
  --local aid = qspawn("child", "test", {id = 2000});
  --send(aid, {id = "send"});
  qnode_tcp_listen(22880, accept);
end

_G["server"] = server
