local server = {}

local cache = {}

require("util")

server.storage = function (_args)
  qlog("in storage")
  while true do
    local arg = qnode_recv()
    qlog("after storage")
    for k, v in pairs(arg) do
      qlog("k: " .. k .. ", v: " .. v)
    end

    local key = arg.key
    local cmd = arg.cmd
    if not key then
      return server.storage()
    end

    if cmd == "set" then
      cache[key] = arg.value
      qlog("set " .. key .. ":" .. arg.value)
      local data = {}
      data.response = "STORED\r\n"
      qnode_send(arg.src, data)
    elseif cmd == "get" then
      local data = {}
      data.val = cache[key]
      data.response = ""
      if data.val then
	qlog("111before send")
	data.response = "VALUE " .. key .. " 0 " .. tostring(string.len(data.val)) .. "\r\n"
	qlog("222before send")
	qlog(data.val)
	data.response = data.response .. data.val .. "\r\n"
	qlog("333before send")
      end
      data.response = data.response .. "END\r\n"
      qlog("before send")
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

