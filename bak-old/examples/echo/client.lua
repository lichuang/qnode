require("util")

local server = {}

server.start = function()
  qlog("echo client start");

  -- accept connection
  local socket, ret = qltcp_connect("0.0.0.0", 22881);
  if socket then
    local out = qltcp_outbuf(socket);
    qlbuffer_write_string(out, "hello");
    local nret, reason = qltcp_send(socket)
    local ret, err = qltcp_recv(socket)
    buffer, ret = qltcp_inbuf(socket)
    local tmp = qlbuffer_get(buffer, 0)
    qlog("recv: " .. tmp)
    qlnode_exit()
  else
    qerror(ret)
  end
end

_G["server"] = server
