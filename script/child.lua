local server = {}

local COMMAND_TOKEN = 1
local KEY_TOKEN     = 2
local TOKEN_FLAGS   = 3
local TOKEN_EXPIRY  = 4
local TOKEN_VLEN    = 5

local NREAD_ADD     = 1
local NREAD_SET     = 2
local NREAD_REPLACE = 3
local NREAD_APPEND  = 4
local NREAD_PREPEND = 5
local NREAD_CAS     = 6

function tokenize_command(_buffer)
  local pos = 0
  local start = 0
  local tokens = {}
  local length = 0
  local c = ""
  local left = 0

  length = qnode_buffer_length(_buffer)

  while pos < length do
    c = qnode_buffer_get(_buffer, pos, 1)

    if c == " " then
      if start ~= pos then
        qnode_buffer_set(buffer, pos, "\0")
        local str = qnode_buffer_get(_buffer, start, pos - start)
        local data = {}
        data.value  = str
        data.length = pos - start
        table.insert(tokens, data)
      end

      start = pos + 1
    elseif c == "\n" then
      if qnode_buffer_get(_buffer, pos - 1, 1) == "\r" then
        pos = pos - 1
        qnode_buffer_set(_buffer, pos, "\0")
      end
      
      left = length - pos - 2
      break
    end

    pos = pos + 1
  end

  if start ~= pos then
    local str = qnode_buffer_get(_buffer, start, pos - start)
    local data = {}
    data.value  = str
    data.length = pos - start
    table.insert(tokens, data)
  end

  print("left " .. left)
  return tokens, left, pos + 2
end

function process_update_command(_buffer, _tokens, _ntokens, _comm, _handle_cas, _left, _pos)
  local key = ""
  local nkey = 0
  local flag = 0
  local exptime = 0
  local vlen = 0
  local data = {}
  local c = ""
  local start = 0

  key     = _tokens[KEY_TOKEN].value
  nkey    = _tokens[KEY_TOKEN].length

  flag    = qnode_strtoul(_tokens[TOKEN_FLAGS].value)
  exptime = qnode_strtoul(_tokens[TOKEN_EXPIRY].value)
  vlen    = qnode_strtoul(_tokens[TOKEN_VLEN].value)

  data.cmd      = "set"
  data.key      = key
  data.flag     = flag
  data.exptime  = exptime
  data.vlen     = vlen

  if _left == vlen + 2 then
    print("vlen: " .. vlen)
    length = qnode_buffer_length(_buffer)

    start = _pos
    while _pos < length do
      c = qnode_buffer_get(_buffer, _pos, 1)
      if c == "\r" then
        qnode_buffer_set(_buffer, _pos, "\0")
        local str = qnode_buffer_get(_buffer, start, _pos - start)
        print("value: " .. str)
        data.value = str
        return data
      end

      _pos = _pos + 1
    end
  end
end

function process_command(_buffer, _tokens, _left, _pos)
  local ntokens = #_tokens

  --[[
  for i, data in ipairs(_tokens) do
    print(i .. " : ".. data.value .. ", len: " .. data.length)
  end
  ]]

  print("command: " .. _tokens[COMMAND_TOKEN].value)
  if _tokens[COMMAND_TOKEN].value == "set" then
    return process_update_command(_buffer, _tokens, ntokens, NREAD_SET, false, _left, _pos)
  end
end

server.child = function (_args)
  print("in child")
  local socket        = _args["sock"]
  local storage_id    = _args["storage_id"]
  -- attach the socket to the actor
  qnode_attach(socket)
  -- recv data from the socket
  qnode_tcp_recv(socket)
  print("after recv")
  buffer = qnode_tcp_buffer(socket)

  local tokens, left, pos = tokenize_command(buffer)
  local data = process_command(buffer, tokens, left, pos)

  --[[
  print("out of process_command")
  for k, v in pairs(data) do
    print(k .. " : ".. v)
  end
  ]]

  if data ~= nil then
    qnode_send(storage_id, data)
  end

  qnode_tcp_send(socket)
end

_G["child"] = server
