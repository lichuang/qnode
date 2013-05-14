local server = {}

local cache = {}

server.storage = function (_args)
  arg = qnode_recv()
  for k, v in pairs(arg) do
    print("k: " .. k .. ", v: " .. v)
  end

  local key = arg.key
  local cmd = arg.cmd
  if not key then
    return
  end

  if cmd == "set" then
    cache[key] = arg
    print("set " .. key .. ":" .. arg.value)
  end
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
