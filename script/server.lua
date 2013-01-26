local server = {}

server.child = function (_sock)
  print("in child: id " .. _sock["id"])
  qnode_tcp_recv(_sock["sock"])
  print("after child")
end

function accept(_listen)
  print("in accept")
  local fd = qnode_tcp_accept(_listen)
  --local aid = qnode_spawn("server", "child", {id = 200, sock = socket})
  print("after accept " .. fd)
end

server.start = function()
  print("server start");
  --local aid = qspawn("child", "test", {id = 2000});
  --send(aid, {id = "send"});
  local socket = qnode_tcp_listen(22880);
  accept(socket)
end

_G["server"] = server
