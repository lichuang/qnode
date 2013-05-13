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

function accept(_listen, _storage_id)
  print("in accept")
  local socket = qnode_tcp_accept(_listen)
  -- spawn a child to handle the request
  local aid = qnode_spawn("child", "child", {sock = socket, storage_id = _storage_id});
  accept(_listen, _storage_id)
end

server.start = function()
  print("server start");

  -- spawn storage process
  local storage_id = qnode_spawn("server", "storage")

  -- accept connection
  local socket = qnode_tcp_listen(22880);
  accept(socket, storage_id)
end

_G["server"] = server
