local server = {}

function tokenize_command(_buffer)
  print("tokenize_command")
  local pos = 0
  local start = 0
  local tokens = {}
  while true do
    pos = qnode_buffer_find(_buffer, start, " ")
    if (pos == -1) then
      break
    end
    print("pos = " .. pos)
    qnode_buffer_set(buffer, pos, "\0")
    local str = qnode_buffer_get(buffer, start, pos - start)
    print("str = " .. str)
    start = pos + 1

    table.insert(tokens, str)
  end
end

server.child = function (_args)
  print("in child")
  local socket = _args["sock"]
  local aid    = _args["aid"]
  qnode_send(aid, {key = "val111222"})
  qnode_attach(socket)
  qnode_tcp_recv(socket)
  print("after recv")
  buffer = qnode_tcp_buffer(socket)

  local tokens = tokenize_command(buffer)
  qnode_tcp_send(socket)
end

_G["child"] = server
