local server = {}

server.storage = function (_args)
  print("in child2")
  arg = qnode_recv()
  print("out child2... " .. type(arg))
  for k, v in pairs(arg) do
    print("k: " .. k .. ", v: " .. v)
  end
  print("out child2" .. arg["key"])
end

function accept(_listen, _aid)
  print("in accept")
  local socket = qnode_tcp_accept(_listen)
  -- spawn a child to handle the request
  local aid = qnode_spawn("child", "child", {sock = socket, aid = _aid});
  accept(_listen, _aid)
end

server.start = function()
  print("server start");

  -- spawn storage process
  local aid = qnode_spawn("server", "storage")

  -- accept connection
  local socket = qnode_tcp_listen(22880);
  accept(socket, aid)
end

_G["server"] = server
