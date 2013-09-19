require("util")

local server = {}

function accept(_listen)
  while true do
    local socket = qltcp_accept(_listen)
    -- spawn a child to handle the request
    local aid = qlnode_spawn("server", "child", {sock = socket});
  end
end

server.test_timer = function ()
  print("test_timer")
end

function main_loop(_args)
  local socket        = _args["sock"]
  -- recv data from the socket
  while true do
    local ret, err = qltcp_recv(socket)
    if not ret then
      qlerror(err)
      qlog("node exit");
      qlnode_exit()
      return
    end
    buffer, ret = qltcp_inbuf(socket)
    if buffer == nil then
      qlerror("reason: " .. ret)
      qlnode_exit()
      return
    end

    local tmp = qlbuffer_get(buffer, 0)
    qlog("buffer: " .. tmp)
    qlbuffer_reset(buffer);
    local out = qltcp_outbuf(socket);
    qlbuffer_write_string(out, tmp);
    local nret, reason = qltcp_send(socket)
    if not nret then
      qlog("qtcp_send error: " .. reason)
      qlnode_exit()
    else
      qlog("qtcp_send: " .. tostring(nret))
    end
  end
end

server.child = function (_args)
  local socket        = _args["sock"]
  -- attach the socket to the actor
  qlnode_attach(socket)

  main_loop(_args)
end

server.start = function()
  qlog("echo server start");

  -- accept connection
  local socket, ret = qltcp_listen(3333);
  if socket then
    local idx, err = qltimer_add(1000, 1000, "server", "test_timer", {})
    if idx then
      qllog("timer id: %d", idx)
    else
      qlerror(err)
    end
    accept(socket)
  else
    qerror(ret)
  end
end

_G["server"] = server
