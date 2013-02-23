local server = {}

server.child = function (_args)
  print("in child")
  local socket = _args["sock"]
  qnode_attach(socket)
  qnode_tcp_recv(socket)
  print("after recv")
  buffer = qnode_tcp_buffer(socket)
  pos = qnode_buffer_find(buffer, "\r\n")
  print("pos = " .. pos)
  qnode_tcp_send(socket)
end

server.child2 = function (_args)
  print("in child2")
  arg = qnode_recv()
  print("out child2")
end

function accept(_listen)
  print("in accept")
  local socket = qnode_tcp_accept(_listen)
  local aid = qnode_spawn("server", "child", {sock = socket});
  accept(_listen)
end

server.start = function()
  print("server start");
  --local socket = qnode_tcp_listen(22880);
  --accept(socket)
  local aid = qnode_spawn("server", "child2")
  qnode_send(aid, {key="val"})
end

_G["server"] = server
