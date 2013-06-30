local server = {}

local cache = {}

require("util")

server.storage = function (_args)
  print("in storage")
  while true do
    local arg = qnode_recv()
    print("after storage")
    for k, v in pairs(arg) do
      print("k: " .. k .. ", v: " .. v)
    end

    local key = arg.key
    local cmd = arg.cmd
    if not key then
      return server.storage()
    end

    if cmd == "set" then
      cache[key] = arg
      print("set " .. key .. ":" .. arg.value)
      local data = {}
      data.response = "STORED\r\n"
      qnode_send(arg.src, data)
    elseif cmd == "get" then
      local data = {}
      --data.val = cache[key]
      data.val = "aaabbb"
      print("111before send")
      data.response = "VALUE " .. key .. " 0 " .. tostring(6) .. "\r\n"
      print("222before send")
      data.response = data.response .. data.val .. "\r\n"
      print("333before send")
      data.response = data.response .. "END\r\n"
      print("before send")
      qnode_send(arg.src, data)
    end
  end
end

function accept(_listen, _storage_id)
  qlog("in accept");
  local socket = qtcp_accept(_listen)
  -- spawn a child to handle the request
  local aid = qnode_spawn("child", "child", {sock = socket, storage_id = _storage_id});
  accept(_listen, _storage_id)
end

server.start = function()
  qlog("server start");

  -- spawn storage process
  local storage_id = qnode_spawn("server", "storage")

  -- accept connection
  local socket = qtcp_listen(22880);
  accept(socket, storage_id)
end

_G["server"] = server

