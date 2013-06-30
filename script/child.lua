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

  length = qbuffer_length(_buffer)
  qlog("get length:" .. length)

  while pos < length do
    c = qbuffer_get(_buffer, pos, 1)
    qlog("get c:" .. c)

    if c == " " then
      if start ~= pos then
        qbuffer_set(_buffer, pos, "\0")
        local str = qbuffer_get(_buffer, start, pos - start)
        local data = {}
        data.value  = str
        data.length = pos - start
        table.insert(tokens, data)
      end

      start = pos + 1
    elseif c == "\n" then
      if qbuffer_get(_buffer, pos - 1, 1) == "\r" then
        pos = pos - 1
        qbuffer_set(_buffer, pos, "\0")
      end
      
      left = length - pos - 2
      break
    end

    pos = pos + 1
  end

  if start ~= pos then
    local str = qbuffer_get(_buffer, start, pos - start)
    local data = {}
    data.value  = str
    data.length = pos - start
    table.insert(tokens, data)
  end

  qlog("left " .. left)
  return tokens, left, pos + 2
end

function process_update_command(_buffer, _tokens, _ntokens,
                                _comm, _handle_cas, _left, _pos)
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

  flag    = qstring_toul(_tokens[TOKEN_FLAGS].value)
  exptime = qstring_toul(_tokens[TOKEN_EXPIRY].value)
  vlen    = qstring_toul(_tokens[TOKEN_VLEN].value)

  data.cmd      = "set"
  data.key      = key
  data.flag     = flag
  data.exptime  = exptime
  data.vlen     = vlen

  if _left == vlen + 2 then
    qlog("vlen: " .. vlen)
    length = qbuffer_length(_buffer)

    start = _pos
    while _pos < length do
      c = qbuffer_get(_buffer, _pos, 1)
      if c == "\r" then
        qbuffer_set(_buffer, _pos, "\0")
        local str = qbuffer_get(_buffer, start, _pos - start)
        qlog("value: " .. str)
        data.value = str
        return data
      end

      _pos = _pos + 1
    end
  end
end

function process_get_command(_buffer, _tokens, _ntokens)
  for i, data in ipairs(_tokens) do
    qlog(i .. " : ".. data.value .. ", len: " .. data.length)
  end
  local i = 2
  local data = {}
  data.cmd = "get"
  while i <= _ntokens do
    data.key = _tokens[i].value
    --qnode_send(storage_id, data)
    --local arg = qnode_recv()
    qlog("data: " .. data.key)
    
    i = i + 1
  end

  return data
end

function process_command(_buffer, _tokens, _left, _pos)
  local ntokens = #_tokens

  --[[
  for i, data in ipairs(_tokens) do
    qlog(i .. " : ".. data.value .. ", len: " .. data.length)
  end
  ]]

  qlog("command: " .. _tokens[COMMAND_TOKEN].value)
  local cmd = _tokens[COMMAND_TOKEN].value
  if cmd == "set" then
    return process_update_command(_buffer, _tokens, ntokens,
                                  NREAD_SET, false, _left, _pos)
  end

  if cmd == "get" then
    return process_get_command(_buffer, _tokens, ntokens)
  end
end

function main_loop(_args)
  local socket        = _args["sock"]
  local storage_id    = _args["storage_id"]
  -- recv data from the socket
  while true do
    local aid = qnode_self();
    qlog("child " .. tostring(aid))
    local buffer, ret = qtcp_recv(socket)
    if buffer == nil then
      qlog("reason: " .. ret)
      qnode_exit()
      return
    end

    local tmp = qbuffer_get(buffer, 0)
    qlog("buffer: " .. tmp)
    local tokens, left, pos = tokenize_command(buffer)
    local data = process_command(buffer, tokens, left, pos)

    qlog("out of process_command")
    for k, v in pairs(data) do
      qlog(k .. " : ".. v)
    end

    if data ~= nil then
      qlog("cmd: " .. data.cmd)
      qnode_send(storage_id, data)
      local arg = qnode_recv()
      for k, v in pairs(arg) do
	qlog("response k: " .. k .. ", v: " .. v)
      end

      qbuffer_reset(buffer);
      local out = qtcp_outbuf(socket);
      qbuffer_write_string(out, arg.response);
      local nret, reason = qtcp_send(socket)
      if not nret then
	qlog("qtcp_send error: " .. reason)
      else
	qlog("qtcp_send: " .. tostring(nret))
      end
    end
  end
end

server.child = function (_args)
  local socket        = _args["sock"]
  local storage_id    = _args["storage_id"]
  -- attach the socket to the actor
  qnode_attach(socket)

  main_loop(_args)
end

_G["child"] = server
