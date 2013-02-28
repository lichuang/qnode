local server = {}

server.child = function (_args)
  print("in child")
  local socket = _args["sock"]
  local aid    = _args["aid"]
  qnode_send(aid, {key = "val111222"})
  qnode_attach(socket)
  qnode_tcp_recv(socket)
  print("after recv")
  buffer = qnode_tcp_buffer(socket)
  pos = qnode_buffer_find(buffer, "*")
  print("pos = " .. pos)
  qnode_tcp_send(socket)
end

_G["child"] = server
