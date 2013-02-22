local server = {}

server.child = function (_args)
  print("in child")
  local socket = _args["sock"]
  qnode_attach(socket)
  qnode_tcp_recv(socket)
  print("after recv")
  qnode_tcp_send(socket)
end

function accept(_listen)
  print("in accept")
  local socket = qnode_tcp_accept(_listen)
  local aid = qnode_spawn("server", "child", {sock = socket});
  accept(_listen)
end

server.start = function()
  print("server start");
  local socket = qnode_tcp_listen(22880);
  accept(socket)
end

_G["server"] = server
